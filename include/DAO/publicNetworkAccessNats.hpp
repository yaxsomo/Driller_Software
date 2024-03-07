/**
 * @file publicNetworkAccessNats.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef PUBLIC_NETWORK_ACCESS_NATS_H
#define PUBLIC_NETWORK_ACCESS_NATS_H

#include <string>
#include "publicNetworkAccess.hpp"
#include <map>

#include "nats/nats.h"


class PublicNetworkAccessNats : public PublicNetworkAccess {
    
    public:
        /**
         * @brief Construct a new Public Network Access Nats object
         * 
         * @param url URL of the NATS server
         */
        PublicNetworkAccessNats(std::string url);

        /**
         * @brief Destroy the Public Network Access Nats object
         */
        ~PublicNetworkAccessNats();

        /**
         * @brief Initialize Nats connection
         * 
         * @return int 
         */
        void init();

        /**
         * @brief Method publishing a message on specified topic (or queue)
         * 
         * @param subject Topic on which the message will be published
         * @param msg Message to be published
         */
        void publish(std::string subject, json msg) ;

        /**
         * @brief Publish a Nats request (waiting for an answer)
         * 
         * @param subject Subject on which the request is published
         * @param msg message of the request
         * @param cb Callback called when the reply is received
         */
        void publishRequest(std::string subject, json msg, boost::function<void (std::string const &)> cb);

        /**
         * @brief Method subscribing to a subject.
         * @todo alex : change Queue to subject
         * @param subject 
         * @param cb 
         */
        void subscribeToSubject(std::string subject, boost::function<void (std::string const &)> cb);

        /**
         * @brief Unsubcribe to specified subject
         * 
         * @param subject Subject name to unsubscribe to
         */
        void unsubscribeSubject(std::string subject);

    private:
        /**
         * @brief Descriptor of the Nats server connexion
         */
        natsConnection *mNatsConnector;

        /**
         * @brief URL of the Nats server to connect to
         */
        std::string mNatsUrl;
        
        /**
         * @brief Status of the Nats connection
         */
        natsStatus mStatus;

        /**
         * @brief Map storing callbacks for each subject
         */
        std::map<std::string, boost::function<void (std::string const &) > > subscriptionCallbacks;

        /**
         * @brief Map storing callbacks for the requests 
         */
        std::map<std::string, natsSubscription*> subscriptionsMap;
        
        /**
         * @brief Static method handling all messages received from the nats server
         * This method needs to be static because the callback is fed to a C language api. 
         * This cannot be achieved by a non-static method. The context is passed through the 
         * closure parameter as a void* and enables us to call the non-static message handler (onMsg)
         * 
         * @param nc descriptor of the nats connector
         * @param sub descriptor of the nats subscriber
         * @param msg message received
         * @param closure user parameter to pass context to the callback. we use this to pass the instance of
         *                our object so that we can then call a non-static member function.
         */
        static void onMsgWrapper(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure);

        /**
         * @brief Non static callback called from the static one
         * 
         * @param nc descriptor of the nats connector
         * @param sub descriptor of the nats subscriber
         * @param msg message received
         */
        void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg);
};

#endif //PUBLIC_NETWORK_ACCESS_NATS_H
