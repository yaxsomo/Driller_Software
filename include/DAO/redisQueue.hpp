/**
 * @file redisQueue.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef REDIS_QUEUE_H
#define REDIS_QUEUE_H

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>
#include <boost/function.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

/** RedisQueue
 * 
 * This is the object handling the reception of messages coming from Redis.
 * When a queue is created, a thread is created and this thread is calling the callback 
 * handling the message.
 */
class RedisQueue
{
    public:
        // Constructor
        RedisQueue(std::string address, std::string port, std::string queueName, 
                   boost::function<void (std::vector<std::string> const &)>cb) : 
            mQueueName(queueName), mPort(port), mAddress(address), mStopped(false), mMessageCb(cb) {};

        void queueSubscriberThread();

        // Stop subscription
        void stopQueue();

    private:
        std::shared_ptr<std::iostream> mStream;
        std::string mQueueName;
        std::string mPort;
        std::string mAddress;
        bool mStopped;

        // Pointer to callback
        boost::function<void (std::vector<std::string> const &)> mMessageCb;
};

#endif //REDIS_QUEUE_H