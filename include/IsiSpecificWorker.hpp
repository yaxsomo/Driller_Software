/**
 * @file IsiSpecificWorker.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef ISIWORKER_H
#define ISIWORKER_H

#include <string>
#include <vector>
#include <boost/function.hpp>
#include <map>
#include <memory>
#include "JSON/json.hpp"

#include "DAO/privateNetworkAccess.hpp"
#include "DAO/bullWorker.hpp"
#include "IsiStats.hpp"

#define STATS_QUEUE_NAME "ISI.STAT"

using json = nlohmann::json;

/** IsiSpecificWorker
 * 
 * This class is handling the job's specific mechanics. Inside Isi, Jobs can be assined to specific agents by 
 * publishing a job order on the topic that is named after this agent. Ex : RobotType:UUID. 
 * 
 * An isiConnect object can add some jobs to his specific worker. When a job order is received, the corresponding 
 * callback function is then called. 
 */
class IsiSpecificWorker {
    public:
        /** Constructor
         * 
         * @param isiName ISI identifier of the IsiConnect
         */
        IsiSpecificWorker(std::string isiName, std::shared_ptr<PrivateNetworkAccess> netAccess, bool isEmergency = false);

        /**
         * @brief Destroy the Isi Specific Worker object
         * 
         */
        ~IsiSpecificWorker() = default;

        /**
         * @brief Launches the worker thread
         * 
         */
        void initAndLaunch();

        /**
         * @brief Stops the worker
         * 
         */
        void stop();

        /** AddJob
         * @brief Method adding a job to the worker.
         * @param name Name of the job to be added
         * @param cb callback function exectued to perform the job.
         */
        void addJob(std::string name, boost::function<json (json const &, const std::string&, IsiStats&)>cb);

        /** removeJob
         * @brief Method removing a job.
         * @param name Name of the job to be removed from the worker
         */
        void removeJob(std::string name);

        /**
         * @brief Adds a callback function to the emergency callback map
         * 
         * @param name Name of the emergency job
         * @param cb Callback function to be called when the job is received
         */
        void addEmergencyJob(std::string name, boost::function<json (json const &, const std::string&)>cb);

        /** removeEmergencyJob
         * @brief Method removing a job.
         * @param name Name of the job to be removed from the worker
         */
        void removeEmergencyJob(std::string name);

        /**
         * @brief 
         * 
         */
        void onTaskProgress(std::string jobId, unsigned int progress, json jobData);

        /**
         * @brief Adds a log to the job
         * 
         * @param jobId Job Id of the Task
         * @param log Log string to be added to the job
         */
        void logTask(std::string jobId, std::string log);

    private:
        /**
         * @brief ISI identifier of the IsiConnect
         * 
         */
        std::string mQueueName;

        /**
         * @brief Boolean indicating if the worker is an emergency worker
         * If it is an emergency worker, it will only use the emergency callback map. 
         * If not, it will use the regular callback map.
         */
        bool mIsEmergencyWorker;

        /** process
         * @brief Private method called when a message is received from the private network. 
         * This message is parsed in order to call the callback function corresponding to the job
         * order received in the message.
         */
        json processJob(const json &jobData, const std::string &jobId);

        /**
         * @brief Processes an emergency job
         * 
         * @param jobData Data of the job
         * @param jobId Id of the job
         * @return json Status of the job
         */
        json processEmergencyJob(const json &jobData, const std::string &jobId);

        /**
         * @brief Compaible bullmq worker
         * 
         */
        std::shared_ptr<BullWorker> bullWorker;

        /**
         * @brief Private network access object
         * 
         */
        std::shared_ptr<PrivateNetworkAccess> mPrivateNetworkAccess;

        /** mJobCallbacks
         * Map containing all the jobs of the worker and the corresponding handling callback functions
         * called when a job order is received.
         */
        std::map<std::string, boost::function<json (json const &, const std::string&, IsiStats&)> > mJobCallbacks;

        /** mEmergencyJobCallbacks
         * Map containing all the jobs of the emergency worker and the corresponding handling callback functions
         * called when a job order is received.
         */
        std::map<std::string, boost::function<json (json const &, const std::string&)> > mEmergencyJobCallbacks;

};

#endif