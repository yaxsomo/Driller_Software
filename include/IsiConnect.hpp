/**
 * @file IsiConnect.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef ISICONNECT_H
#define ISICONNECT_H

#include "DAO/privateNetworkAccessBullRedis.hpp"
#include "DAO/publicNetworkAccessNats.hpp"
#include "IsiSpecificWorker.hpp"
#include "IsiGenericWorker.hpp"
#include "DAO/bullWorker.hpp"
#include "utils/macAddr.hpp"
#include "IsiHeartbeat.hpp"
#include "sharedContext.hpp"
#include "isiAlarm.hpp"
#include "dotenv.h"

#include <memory>

//class IsiSpecificWorker;

using namespace dotenv;

// For logging
#define BOOST_LOG_DYN_LINK 1

/**
 * IsiConnect
 * @brief Main object used to create an IsiConnect agent. 
 * In order to implement a new IsiAgent, create an object that inherits and implements
 * this IsiConnect class.
 * 
 * Then add jobs with their callbacks and you're good to go !
 */
template <class T>
class IsiConnect {
    public:
        /**
         * @brief Construct a new Isi Connect object
         * 
         * @param robotType Type of the robot used for generic jobs
         * @param uuid unique identifier used to build the robots unique name and to receive
         *             specific job orders
         */
        IsiConnect(std::string robotType, std::string uuid="")
        {
            std::string id;

            // First check that robotType doesn't contain any charcaters that are not alphanumeric or underscore
            if (robotType.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos)
            {
                BOOST_LOG_TRIVIAL(error) << "Robot type contains invalid characters. Only alphanumeric and underscore are allowed.";
                exit(1);
            }
            
            // Load environment variables
            env.load_dotenv(".env", false);
            checkEnvironmentVariables();

            // Instanciate Internal communication using Redis
            mInternalCommunicator = std::make_shared<PrivateNetworkAccessBullRedis>(env["REDIS_ADDRESS"], env["REDIS_PORT"]);

            // Instanciate External communication using Nats
            mExternalCommunicator = std::make_shared<PublicNetworkAccessNats>(env["NATS_URL"]);

            if (uuid == "")
            {
                try {
                    // Get mac address as UUID
                    id = getMacAddress(env["NETWORK_IFACE"]);
                } catch(std::invalid_argument e) {
                    // We want the program to stop, so rethrow exception
                    BOOST_LOG_TRIVIAL(error) << e.what();
                    throw(e);
                }
            } else {
                id = uuid;
            }

            // TODO : get those from the env
            mIsiName = robotType + "." + id;
            BOOST_LOG_TRIVIAL(info) << "Creating Isiconnect object with name " << mIsiName;
        }

        /**
         * @brief Destroy the Isi Connect object
         * 
         */
        ~IsiConnect()
        {
            // Todo @alex : convert everything to smart pointers
            // Stop Worker threads
            mSpecificWorker->stop();
            mEmergencyStopWorker->stop();
            mHeartbeat->stop();
        }

        /**
         * @brief Method initializing all the IsiConnect objects and creating all the connections
         * 
         */
        void init()
        {
            // Init all the communicators
            mInternalCommunicator->init();
            mExternalCommunicator->init();

            // Setup all queues
            setupQueues();

            // Setup all workers
            setupWorkers();    
        }

        /**
         * @brief Method returning the availability status of the IsiConnect object.
         * The availability is the capability of the IsiConnect object to take a job at a given moment.
         * This method is pure virtual and must be implemented in the IsiConnect inheriting object.
         * @return true IsiConnect object is free to take a job order
         * @return false IciConnect is bust and cannot take a job order
         */
        virtual bool isAvailable() = 0;

        /**
         * @brief Method stopping all systems on the IsiConnect object.
         * This method must be implemented in the IsiConnect inheriting object. The goal is to perform
         * a specific emergency stop on all systems of the IsiConnect object.
         */
        virtual json emergencyStop(json data, std::string jobId) = 0;

        /**
         * @brief Publish an alarm that will be received by the core
         * 
         * @param name name of the alarm
         * @param description description of the alarm
         * @return std::string UUID of the alarm
         */
        std::string publishAlarm(std::string name, std::string description)
        {
            return mAlarms->publishNewAlarm(name, description);
        }

        /**
         * @brief Acknowledge an alarm
         * 
         * @param alarmId Id of the alarm to acknowledge
         */
        void ackAlarm(std::string alarmUUID)
        {
            mAlarms->ackAlarm(alarmUUID);
        }

        /**
         * @brief Method adding a job to the isiConnect object
         * @param name Name of the job
         * @param cb Callback called to execute the job
         */
        void addJob(std::string name, boost::function<json (json const &, const std::string&, IsiStats&)>cb)
        {
            // Add job to the internal communication system
            mSpecificWorker->addJob(name, cb);
        }

        /**
         * @brief Removes the job from the specific worker.
         * 
         * @param name Name of the job to remove
         */
        void removeJob(std::string name)
        {
            mSpecificWorker->removeJob(name);
        }

        /**
         * @brief Updates the core about the progress percentage of the task.
         * 
         * @param jobId Job Id of the task
         * @param progress Percentage of progress
         */
        void updateProgress(std::string jobId, unsigned int progress, json jobData)
        {
            mSpecificWorker->onTaskProgress(jobId, progress, jobData);
        }

        /**
         * @brief Subscribe to a topic on the external networking protocol
         * @param topic topic name to subscribe to
         * @param cb callback function called when a message is received on this topic
         */
        void subscribeToExternalTopic(std::string topic, boost::function<void (std::string const&)>cb)
        {
            mExternalCommunicator->subscribeToSubject(topic, cb);
        }

        /**
         * @brief Subscribe to a request topic on the external communcation network
         * @param topic topic name of the request
         * @param cb callback called on request
        */
        void subscribeToRequest(std::string topic, boost::function<void (std::string const&)>cb)
        {
            mExternalCommunicator->subscribeToSubject(mIsiName + '.' + topic, cb);
        }

        /**
         * @brief Publish a Json message on the external communication network
         * 
         * @param topic Topic on which to send the message
         * @param msg JSON message to be sent
         */
        void publishExternalMsg(std::string topic, json msg)
        {
            mExternalCommunicator->publish(topic, msg);
        }

        /**
         * @brief Publish a message on the external communication network. The topic is fixed with the IsiName
         * of the object.
         * 
         * @param msg Message to be sent
         */
        void publishFeedback(json msg)
        {
            mExternalCommunicator->publish(mIsiName + ":feedback", msg);
        }

        /**
         * @brief Publish a message on the external communication network. The topic is modified with the Isiname.
         * The topic is as follows : $Isiname:topic
         * @param topic topic to be added to the isiName to form the complete topic name
         * @param msg Message to be sent
         */
        void publishSpecificFeedback(std::string topic, json msg)
        {
            mExternalCommunicator->publish(mIsiName + ":feedback:" + topic, msg);
        }

        /**
         * @brief Adds a log to the task log list
         * 
         * @param jobId Job ID of the task
         * @param log log to be added
         */
        void logTask(std::string jobId, std::string log)
        {
            mSpecificWorker->logTask(jobId, log);
        }

    private:
        /**
         * @brief Object handling the internal communication with IsiCore
         */
        std::shared_ptr<PrivateNetworkAccess> mInternalCommunicator;
        
        /**
         * @brief Object handling the external comminication with other IsiConnect object or IsiVue
         */
        std::shared_ptr<PublicNetworkAccess> mExternalCommunicator;

        /**
         * @brief Context to be shared with other objects
         * @todo not so usefull... remove maybe
         */
        SharedContext mSharedContext;

        /**
         * @brief Specific worker handling jobs ordered specifically to this IsiConnect object.
         * 
         */
        std::shared_ptr<IsiSpecificWorker> mSpecificWorker;

        /**
         * @brief Specific worker handling emergency stop jobs
         * 
         */
        std::shared_ptr<IsiSpecificWorker> mEmergencyStopWorker;

        /**
         * @brief Generic worker handling jobs common to all robots that have the same type
         * 
         */
        IsiGenericWorker *mGenericWorker;

        //IsiWorker *mEmergencyWorker;

        /**
         * @brief Object sending heartbeats to the core
         */
        std::shared_ptr<IsiHeartbeat> mHeartbeat;

        /**
         * @brief Object sending alarms to the core
         */
        std::shared_ptr<IsiAlarm> mAlarms;

        /**
         * @brief Name of the IsiConnect object
         * This name is the concatenation of the robot type and the robot's UUID.
         * Ex : LaserCutter:abcd1234
         */
        std::string mIsiName;

        /**
         * @brief Method checking that all environment variables are set correctly
         */
        void checkEnvironmentVariables()
        {}

        /**
         * @brief Method setuping all IsiConnect's workers.
         */
        void setupWorkers()
        {
            //mSpecificWorker = new IsiSpecificWorker(mInternalCommunicator, mIsiName);
            mSpecificWorker = std::make_shared<IsiSpecificWorker>(mIsiName, mInternalCommunicator);
            mSpecificWorker->initAndLaunch();

            mEmergencyStopWorker = std::make_shared<IsiSpecificWorker>(mIsiName + ".EMERGENCY", mInternalCommunicator, true);
            mEmergencyStopWorker->initAndLaunch();
            mEmergencyStopWorker->addEmergencyJob("stop", boost::bind(&T::emergencyStop, dynamic_cast<T*>(this), boost::placeholders::_1, boost::placeholders::_2));
        }

        /**
         * @brief Setup all isiConnect's queues (alarm, heartbeat...)
         * 
         */
        void setupQueues()
        {
            if(env["HEARTBEAT_PERIOD"] != "")
            {
                mHeartbeat = std::make_shared<IsiHeartbeat>(mInternalCommunicator, mIsiName, std::stoi(env["HEARTBEAT_PERIOD"]));
            } else {
                mHeartbeat = std::make_shared<IsiHeartbeat>(mInternalCommunicator, mIsiName);
            }

            mAlarms = std::make_shared<IsiAlarm>(mIsiName, mInternalCommunicator);
        }
};

#endif //ISICONNECT_H