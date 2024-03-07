/**
 * @file IsiHeartbeat.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef ISIHEARTBEAT_H
#define ISIHEARTBEAT_H

#include <boost/thread.hpp>
#include <string>
#include <memory>

#include "DAO/privateNetworkAccess.hpp"

/**
 * @brief Heartbeat timer perdio in seconds
 * Default is 10 seconds
 */
#define HEARBEAT_TIMER_PERIOD_S 2

#define HEARTBEAT_QUEUE_NAME "ISI.HEARTBEAT"

/**
 * @brief "Worker" sending heartbeats to the Isi Core over the internal communication network.
 * This worker functions inside its own thread.
 */
class IsiHeartbeat 
{
    public:
        /**
         * @brief Construct a new Isi Heartbeat object
         * 
         * @param networkAccess 
         * @param queueName 
         */
        IsiHeartbeat(std::shared_ptr<PrivateNetworkAccess> networkAccess, 
            std::string isiName,
            int period = HEARBEAT_TIMER_PERIOD_S);
        
        /**
         * @brief Destroy the Isi Heartbeat object
         * 
         */
        ~IsiHeartbeat() = default;

        /**
         * @brief Thread sending heartbeat messages periodically
         * 
         */
        void run();

        /**
         * @brief Stops the thread
         * 
         */
        void stop();

    private:
        /**
         * @brief Pointer onto the private network accessor.
         * This is necessary to send heartbeat messages over on the internal network
         * to the Isi Core.
         */
        std::shared_ptr<PrivateNetworkAccess> mPrivateNetAccessor;

        /**
         * @brief Period at which the heartbeat is sent.
         * 
         */
        int mTimerPeriodSecs;

        /**
         * @brief Boolean to stop the thread.
         * 
         */
        bool mRunning;

        /**
         * @brief Name of the IsiConnect object.
         * 
         */
        std::string mIsiName;

        /**
         * @brief Thread sending the heartbeats at the specified period.
         * 
         */
        std::shared_ptr<boost::thread> mHeartbeatThread;
};

#endif