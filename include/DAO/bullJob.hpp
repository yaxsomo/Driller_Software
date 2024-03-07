/**
 * @file bullJob.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef BULL_JOB_HPP
#define BULL_JOB_HPP

#include <string>
#include "JSON/json.hpp"
#include "backoff.hpp"

using json = nlohmann::json;


struct BullJobOptions {
    public:
        BullJobOptions(json jsonData){
            rawJSON = jsonData;
            removeOnFail = rawJSON.value("removeOnFail", false);
            removeOnComplete = rawJSON.value("removeOnComplete", false);
            delay = rawJSON.value("delay", 0);
            attempts = rawJSON.value("attempts", 1);
            
            if(rawJSON.value("backoff", json::object()).is_null())
            {
                backoffDelay = 0;
                backoffType = BackoffStrategy::NONE;
            }
            else
            {
                backoffDelay = rawJSON.value("backoff", json::object()).value("delay", 0);
                std::string backoffTypeStr = rawJSON.value("backoff", json::object()).value("type", "none");
                
                if (backoffTypeStr == "fixed"){
                    backoffType = BackoffStrategy::FIXED;
                }
                else if (backoffTypeStr == "exponential"){
                    backoffType = BackoffStrategy::EXPONENTIAL;
                }
                else{
                    BOOST_LOG_TRIVIAL(error) << "Unknown backoffType in BullJobOptions";
                    backoffType = BackoffStrategy::NONE;
                }
            }            
        };

        BullJobOptions(const BullJobOptions& other) {
            rawJSON = other.rawJSON;
            removeOnFail = other.removeOnFail;
            removeOnComplete = other.removeOnComplete;
            delay = other.delay;
            attempts = other.attempts;
            backoffDelay = other.backoffDelay;
            backoffType = other.backoffType;
        }

        json rawJSON;
        bool removeOnFail;
        bool removeOnComplete;
        unsigned int delay;
        unsigned int attempts;
        unsigned int backoffDelay;
        BackoffStrategy backoffType;
};


/**
 * @brief Data structure to store all the received data from bull on a new job received
 * 
 */
struct BullJob {
    public:
        BullJob(json jsonData, std::string uuid) {
            rawJSON = jsonData;
            uuid = uuid;

            // Get data from json
            priority = rawJSON.value("priority", "0");
            parentKey = rawJSON.value("parentKey", "");
            data = rawJSON["data"];
            opts = std::make_unique<BullJobOptions>(rawJSON["opts"]);
            
            json p = rawJSON.value("parent", json::object());
            if(p.size() == 0)
            {
                parentQueue = "";
                parentFpof = false;
                parentIdIsStr = true;
                parentId = "";
            }
            else
            {
                try
                {
                    parentId = p.value("id", "");
                    parentIdIsStr = true;
                }
                catch(const std::exception& e)
                {
                    // Parent id is an int... 
                    parentIdInt = rawJSON.value("parent", json::object()).value("id", 0);
                    parentIdIsStr = false;
                }
                parentQueue = rawJSON.value("parent", json::object()).value("queueKey", "");
                parentFpof = rawJSON.value("parent", json::object()).value("fpof", false);
            }
            delay = rawJSON.value("delay", "0");
            name = rawJSON.value("name", "");
            if(name == "")
                throw std::runtime_error("Job name is empty");
            attemptsMade = std::stoi(rawJSON.value("attemptsMade", "0"));
            timestamp = rawJSON.value("timestamp", "");
            processedOn = rawJSON.value("processedOn", "");
        };

        /**
         * @brief Construct a new Bull Job object - Copy constructor
         * 
         * @param other BullJob to be copied
         */
        BullJob(const BullJob& other)
        {
            rawJSON = other.rawJSON;
            uuid = other.uuid;

            // Get data from json
            priority = other.priority;
            parentKey = other.parentKey;
            data = other.data;
            opts = std::make_unique<BullJobOptions>(*other.opts);
            parentId = other.parentId;
            parentIdInt = other.parentIdInt;
            parentIdIsStr = other.parentIdIsStr;
            parentQueue = other.parentQueue;
            parentFpof = other.parentFpof;
            delay = other.delay;
            name = other.name;
            attemptsMade = other.attemptsMade;
            timestamp = other.timestamp;
            processedOn = other.processedOn;
        }

        json rawJSON;
        std::string uuid; 
        std::string priority; // Yes, it is received as a string from bull...
        std::string parentKey;
        json data;
        std::unique_ptr<BullJobOptions> opts;
        std::string parentId;
        unsigned int parentIdInt;
        bool parentIdIsStr;
        std::string parentQueue;
        bool parentFpof;
        std::string delay;  // Yes, it is received as a string from bull...
        std::string name;
        unsigned int attemptsMade; // Yes, it is received as a string from bull...
        std::string timestamp;
        std::string processedOn;
};

#endif