/**
 * @file publicNetworkAccess.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef PUBLIC_NETWORK_ACCESS_H
#define PUBLIC_NETWORK_ACCESS_H

#include <boost/function.hpp>
#include <string>
#include "JSON/json.hpp"

using json = nlohmann::json;

/**
 * @brief This is the base class used by the Isi external communication system. 
 * This is an implementation of the DAO pattern. This class is inherited by the specific network used. 
 * This way, if the network protocol is to be changed, a new implementation can easily be fitted as soon as 
 * it implements the virtual methods of this class. 
 */
class PublicNetworkAccess {
    public:
        /**
         * @brief Method initiating all the connections
         */
        virtual void init() = 0;

        /**
         * @brief Create a subscription on a specific topic
         * 
         * @param subject Topic name to subscribe to
         * @param cb Callback called when a message is received on the topic
         */
        virtual void subscribeToSubject(std::string subject, boost::function<void (std::string const &)> cb) = 0;

        /**
         * @brief Publish a message on the specified topic
         * 
         * @param subject Topic to publish the message in
         * @param msg Message to be sent
         */
        virtual void publish(std::string subject, json msg) = 0;

        /**
         * @brief Publish a request (publication with a reply)
         * 
         * @param subject Subject on which request is sent
         * @param msg message of the request
         * @param cb callback called when reply is received
         */
        virtual void publishRequest(std::string subject, json msg, boost::function<void (std::string const &)> cb) = 0;
};


#endif  //PUBLIC_NETWORK_ACCESS_H