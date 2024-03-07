/**
 * @file bullWorker.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef BULL_WORKER_HPP
#define BULL_WORKER_HPP

#include "DAO/privateNetworkAccessBullRedis.hpp"
#include <string>
#include <memory>
#include <boost/thread.hpp>
#include "DAO/bullJob.hpp"

#define DEFAULT_POLLING_TIMER_S 5
#define LOCK_DURATION_S 30

#define ACTION_QUEUE_NAME "action_queue"

/**
 * @brief BullMq-compatible worker
 * 
 */
class BullWorker {
    public:
        /**
         * @brief Construct a new Bull Worker object, creating its own redis connection
         * 
         * @param queueName 
         */
        BullWorker(const std::string& queueName);

        /**
         * @brief Construct a new Bull Worker object, using an already established redis connection
         * 
         * @param connection 
         * @param queueName 
         */
        BullWorker(PrivateNetworkAccess* connection, const std::string& queueName);

        /**
         * @brief Starts the worker thread
         * 
         */
        void initAndLaunch(const boost::function<json (json const &, const std::string&)>& cb);

        /**
         * @brief Stops the worker thread
         * 
         */
        void stopThread();

        /**
         * @brief Updates the core about a job's progress
         * 
         * @param jobId Id of the running job
         * @param progress Percentage of progress
         */
        void onTaskProgress(std::string jobId, unsigned int progress, json jobData);

        /**
         * @brief Adds a log to the job
         * 
         * @param jobId Id of the running job
         * @param log string to be added to the job's log
         */
        void logTask(const std::string& jobId, const std::string& log) const;

        /**
         * @brief Destroy the Bull Worker object
         * 
         */
        ~BullWorker() = default;

    private:
        /**
         * @brief Name of the queue to listen to
         */
        std::string mQueueName;

        /**
         * @brief Boolean to stop the worker thread
         * 
         */
        bool mRunning;

        /**
         * @brief Redis connection accessor
         */
        std::shared_ptr<PrivateNetworkAccessBullRedis> mInternalAccessor;

        /**
         * @brief Main worker thread
         */
        std::shared_ptr<boost::thread> mWorkerThread;

        /**
         * @brief Polling timer for the worker to check if new jobs were added
         */
        uint16_t mTimerPeriodSecs;

        /**
         * @brief function called when a job is received
         * 
         */
        boost::function<json (json const &, const std::string&)> mJobCallback;

        /**
         * @brief Worker thread function
         */
        void run();

        /**
         * @brief Launch the job in a thread, and returns a promise
         * 
         * @param name name of the job
         * @param data data of the job
         * @param jobId id of the job
         * @param promise promise to be fulfilled when the job is done
         */
        void launchJob(const std::string& name, const BullJob& data, const std::string& jobId, boost::promise<json> &promise) const;

        /**
         * @brief Process job when a job is received
         * 
         * @param msg Data of the job to be performed
         * @return Data to be sent back to the core
         */
        json processJob(const std::string& name, BullJob& data, const std::string& jobId, const std::string& lockToken);

        /**
         * @brief Checks if the job is longer than the lock duration, and updates the lock if needed
         * 
         * @param jobId 
         * @param lockToken 
         * @param name 
         * @param t 
         * @return std::chrono::time_point<std::chrono::high_resolution_clock> 
         */
        void extendLockDuringCallback(const std::string& jobId, const std::string& lockToken, const std::string& name, boost::thread *t);

        /**
         * @brief Returns wether the job data is valid or not
         * 
         * @param jobData 
         * @return true 
         * @return false 
         */
        bool isJobDataValid(const std::string& jobData) const;

        /**
         * @brief To be called at the end of a task
         * 
         */
        std::string onTaskCompleted(const std::string& jobId, const BullJob& jobData, const json& status, const std::string& lockToken);
};

#endif //BULL_WORKER_HPP