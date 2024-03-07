/**
 * @file privateNetworkAccess.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef PRIVATE_NETWORK_ACCESS_H
#define PRIVATE_NETWORK_ACCESS_H

#include <vector>
#include <string>
#include <boost/function.hpp>
#include "JSON/json.hpp"

using json = nlohmann::json;

/**
 * @brief This is the base class used by the Isi internal communication system. 
 * This is an implementation of the DAO pattern. This class is inherited by the specific network used. 
 * This way, if the network protocol is to be changed, a new implementation can easily be fitted as soon as 
 * it implements the virtual methods of this class.
 */
class PrivateNetworkAccess {
    protected:
    /**
     * @brief Construct a new Private Network Access object
     * 
     */
    PrivateNetworkAccess(){};

    public: 
        /**
         * @brief Destroy the Private Network Access object
         */
        virtual ~PrivateNetworkAccess(){};

        /**
         * @brief Init method to be called to create all the connections.
         */
        virtual void init() = 0;

        /**
         * @brief Create a subscription to the specified queue.
         * 
         * @param queueName Name of the queue
         * @param cb Callback called when a message is received on this queue
         */
        virtual void subscribeToTopic(std::string queueName, boost::function<void (std::vector<std::string> const &)> cb) = 0;

        /**
         * @brief Closes the subscription to the specified queue
         * 
         * @param queueName Name of the queue to be closed
         */
        virtual void unsubscribeTopic(std::string queueName) = 0;

        /**
         * @brief Publish a message on a queue
         * 
         * @param queueName queue to publish the message in
         * @param msg Message to be sent on the queue
         */
        virtual void publish(std::string queueName, json msg) = 0;

        /**
         * @brief Create a Queue to publish on
         * 
         * @param name Name of the queue
         */
        virtual void createQueue(std::string name) = 0;

        /**
         * @brief Create a job in the specified queue
         * 
         * @param queueName Queue where the job is to be added
         * @param jobId Job name
         * @param isiName IsiName
         * @param data Arbitrary data sent inside the request
         */
        virtual std::string addJob(std::string queueName,
                    std::string jobId,
                    std::string isiName,
                    json data) = 0;
};


#endif //PRIVATE_NETWORK_ACCESS_H