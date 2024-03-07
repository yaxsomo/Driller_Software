/**
 * @file privateNetworkAccessBullRedis.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef PRIVATE_NETWORK_ACCESS_REDIS_H
#define PRIVATE_NETWORK_ACCESS_REDIS_H

#include "DAO/privateNetworkAccess.hpp"

#define REDISCPP_HEADER_ONLY
#include "redis-cpp/stream.h"
#include "redis-cpp/execute.h"

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/log/trivial.hpp>
#include <map>
#include <queue>
#include <utility>

#include "JSON/json.hpp"
#include "DAO/redisQueue.hpp"

using json = nlohmann::json;

#define DEFAULT_REDIS_ADDR "localhost"
#define DEFAULT_REDIS_PORT "6379"
#define BULL_VERSION "3.5.4"

/**
 * @brief Struct wrapping the msgpack serializer. 
 * Used by the add job method. 
 * IMPORTANT : dont change attributes order !
 */
struct AddJobMetadata {
    std::string keyPrefix;
    std::string customId;
    std::string name;
    uint32_t timestamp;
    std::string parentKey;
    std::string waitChildrenKey;
    std::string parentDependancyKey;
    std::string parentIdQueueKey;
    std::string repeatJobKey;

    template<class T>
    void pack(T &pack) {
        pack(keyPrefix, 
            customId, 
            name, 
            timestamp, 
            parentKey, 
            parentKey, 
            waitChildrenKey, 
            parentDependancyKey, 
            parentIdQueueKey, 
            repeatJobKey);
    }
};

/**
 * @brief Struct wrapping the msgpack serialized function for the addjob options.
 * IMPORTANT : dont change attributes order !
 */
struct AddJobOptions {
    std::map<std::string, u_int8_t> attempts;
    std::map<std::string, u_int8_t> delay;
    std::map<std::string, bool> removeOnComplete;
    std::map<std::string, bool> removeOnFail;
    std::map<std::string, std::string> jobId;
    std::map<std::string, std::string> backoff;

    template<class T>
    void pack(T &pack) {
        pack(attempts, delay, removeOnComplete, removeOnFail, 
             jobId, backoff);
    }
};

/**
 * @brief This class implements the ISI internal communications using REDIS.
 * 
 */
class PrivateNetworkAccessBullRedis : public PrivateNetworkAccess {
    public:
        /**
         * @brief Construct a new Private Network Access Redis object
         * 
         */
        PrivateNetworkAccessBullRedis(std::string addr = DEFAULT_REDIS_ADDR, std::string port = DEFAULT_REDIS_PORT);

        /**
         * @brief Destroy the Private Network Access Redis object
         * 
         */
        ~PrivateNetworkAccessBullRedis();

        /**
         * @brief Method launching the Redis publisher thread.
         * 
         */
        void init();

        /**
         * @brief Subscribes to a queue by creating a thread receiving messages
         * from the specified redis queue. All queues and threads are stored
         * in two maps with keys setup with the queue name.
         * 
         * @param queueName Name of the queue to subscribe to
         * @param cb callback called when a message is received
         */
        void subscribeToTopic(std::string queueName, boost::function<void (std::vector<std::string> const &)> cb);
        
        /**
         * @brief Stops the receiving thread related to the specified queue
         * and removes the queue from the map.
         * 
         * @param queueName Queue to be removed.
         */
        void unsubscribeTopic(std::string queueName);

        /**
         * @brief Adds a message to the publishing queue. 
         * The publishing will happen in the publishing thread.
         * 
         * @param queueName Name of the topic or queue to publish in.
         * @param msg JSON message to be published
         */
        void publish(std::string queueName, json msg);

        /**
         * @brief Method stopping the publisher thread
         */
        void stopPublisher();

        /**
         * @brief Creates a Bullmq queue with given name
         * 
         * @param name 
         */
        void createQueue(std::string name);

        /**
         * @brief Adds a bullmq job to a queue
         * 
         * @param queueName 
         * @param jobId 
         * @param isiName name of the isiCOnnect
         * @param data 
         */
        std::string addJob(std::string queueName,
                    std::string jobId,
                    std::string isiName,
                    json data);

        /**
         * @brief Sends a MoveStalledJobsToWait command to bull redis server
         * 
         * @param isiName Isi Name name of the isiCOnnect
         * @return std::string 
         */
        std::string sentMoveStalledJobsToWaitCmd(std::string isiName);

        /**
         * @brief Send a Move To Active script command to redis
         * 
         * @param isiName name of the isiCOnnect
         * @param lockToken UUID used as token for the created lock
         * @return std::string 
         */
        std::string sendMoveToActiveCmd(std::string isiName, std::string lockToken, std::string jobId = "");

        /**
         * @brief Sends a brpoplpush to Redis from the wait key to the worker's active key
         * 
         * @param isiName name of the isiCOnnect
         * @param delay delay for which the active will be set
         */
        std::string brpoplpushToActive(std::string isiName, uint16_t delay);

        /**
         * @brief Moves a job to a finished state using corresponding LUA script
         * 
         * @param isiName name of the isiCOnnect
         * @return std::string 
         */
        std::string MoveJobFromActiveToAFinishedStatus(std::string isiName, std::string jobId, json status, std::string lockToken, int attemptsMade, bool shouldRemove, bool fpof);

        /**
         * @brief Updates the progress on an ongoing job
         * 
         * @param jobID Job being processed
         * @param progress Percentage of progress of the job
         * @return std::string 
         */
        std::string updateProgress(std::string isiName, std::string jobID, unsigned int progress);

        /**
         * @brief Set the Client Name in Redis
         * 
         * @param isiName name of the isiCOnnect
         */
        std::string setClientName(std::string isiName);

        /**
         * @brief Extends the lock of a job
         * 
         * @param queueName Name of the queue containing the job
         * @param jobId Id of the job to extend the lock
         * @param lockDuration Duration of the lock
         * @return std::string 
         */
        std::string extendLock(std::string queueName, std::string jobId, int lockDuration, std::string lockToken);
        
        /**
         * @brief Execute a redis command
         * @note This method has to be implemented inside the .h because it's a template variadic method. Not doing it results in an undefined
         *       reference error at the linking stage.
         * @see https://stackoverflow.com/questions/25091516/c-template-variadic-function-undefined-reference
         * 
         * @param cmd Command to send to redis
         * @return String containing the response of the cmd
         */
        template<typename... Args>
        std::string executeCmd(std::string cmd, Args... args)
        {
            std::string s;
            try
            {
                // Send message to redis
                auto response = rediscpp::execute(*mStream, cmd, std::forward<Args>(args) ...);
                
                // Lambda function to extract data from the array 
                // Note : this is a recursive lambda function. We feed the lambda its own implementation as a parameter.
                auto extract_message = [] (auto const &value, auto& extract_ref) -> std::string
                {
                    using namespace rediscpp::resp::deserialization;
                    std::string s;
                    std::visit(rediscpp::resp::detail::overloaded{
                            [&s] (bulk_string const &val)
                            {
                                s = val.get();
                                // Add "" around the string so that json parsing succeeds
                                if(s.at(0) != '{') {
                                    s.insert(0, 1,'"');
                                    s.append(1, '"');
                                }
                            },
                            [&s] (simple_string const &val)
                            { s = val.get();},
                            [&s] (integer const &val)
                            { s = std::to_string(val.get());},
                            [&s] (error_message const &val)
                            { s = val.get();},
                            [&s, &extract_ref] (array const &arr)
                            {
                                // JSON reconstruction
                                std::vector<std::string> str_arr;
                                std::string str;
                                // Start the json array enclosure with a {
                                str += "{";
                                // Put everything in a vector of strings
                                for (auto const &i : arr.get())
                                    str_arr.push_back(extract_ref(i, extract_ref));

                                int ctr = 0;
                                for(int i = 0; i < str_arr.size(); ++i)
                                {
                                    // Reconstruct each json element
                                    str += str_arr.at(i);
                                    if(ctr++%2 == 0) {
                                        str+=":";
                                    } else {
                                        // If the element is not the last one of the array, add a ","
                                        if(i != (str_arr.size() -  1))
                                            str+=",";
                                    }
                                }
                                // Add a } to close the json array
                                str += "}";
                                s+=str;
                            },
                            [] (auto const &)
                            { BOOST_LOG_TRIVIAL(error) << "Unexpected value type."; }
                        }, value);
                    return s;
                };

                // Visitor to interpret the response depending on the response type
                std::visit(rediscpp::resp::detail::overloaded{
                    [cmd, &extract_message, &s] (rediscpp::resp::deserialization::array const &arr)
                    {
                        std::vector<std::string> str_arr;
                        BOOST_LOG_TRIVIAL(debug) << cmd << " : array received";
                        std::string str;
                        // JSON reconstruction
                        str += "{";
                        for (auto const &i : arr.get())
                            str_arr.push_back(extract_message(i, extract_message));
                        int ctr = 0;
                        std::string tmp;
                        for (auto const &i: str_arr)
                        {
                            // If array size is 2, for an unknown reason, items are reversed, so invert them to fix json parsing
                            if(str_arr.size() == 2)
                            {
                                if(ctr++ == 0) tmp = i;
                                else {
                                    str += i + ":" + tmp;
                                }
                            } else {
                                str += i;
                                if(ctr++%2 == 0) {
                                    str+=":";
                                } else {
                                    if(i != str_arr.at(str_arr.size() -  1))
                                        str+=",";
                                }
                            }
                        }
                        str += "}";
                        s+= str;
                        BOOST_LOG_TRIVIAL(debug) << cmd << ":" << s;
                    },
                    [cmd, &s] (rediscpp::resp::deserialization::bulk_string const &val)
                    {
                        BOOST_LOG_TRIVIAL(debug) << cmd << " : " << val.get();
                        s += val.get();
                    },
                    [cmd, &s] (rediscpp::resp::deserialization::integer const &val)
                    {
                        BOOST_LOG_TRIVIAL(debug) << cmd << " : " << std::to_string(val.get());
                        s += std::to_string(val.get());
                    },
                    [cmd, &s] (rediscpp::resp::deserialization::binary_data const &val)
                    {
                        BOOST_LOG_TRIVIAL(debug) << cmd << " : " << val.get();
                        s += val.get();
                    },
                    [cmd, &s] (rediscpp::resp::deserialization::simple_string const &val)
                    {
                        BOOST_LOG_TRIVIAL(debug) << cmd << " : " << val.get();
                        s += val.get();
                    },
                    [cmd] (rediscpp::resp::deserialization::null const &)
                    {
                        BOOST_LOG_TRIVIAL(debug) << cmd << " : null";
                    },
                    // Oops. An error in a response.
                    [] (rediscpp::resp::deserialization::error_message const &err)
                    { 
                        BOOST_LOG_TRIVIAL(error) << "Error: " << err.get() << std::endl;
                    },
                    // An unexpected response.
                    [] (auto const &)
                    { 
                        BOOST_LOG_TRIVIAL(error) << "Unexpected value type." << std::endl;
                    }
                }, response.get());

                return s;
            }
            catch(const std::exception& e)
            {
            return e.what();
            }
        }


    private:
        /**
         * @brief Adress of the redis server to connect to.
         */
        std::string mRedisAddress;

        /**
         * @brief Port on which to connect to the redis server
         */
        std::string mRedisPort;

        /**
         * @brief iostream used to send data and commands to redis
         */
        std::shared_ptr<std::iostream> mStream;

        /**
         * @brief Map storing all the subscribed queues 
         */
        std::map<std::string, boost::shared_ptr<RedisQueue> > subscriberQueueMap;
        
        /**
         * @brief Map storing all the subscriber threads
         */
        std::map<std::string, boost::shared_ptr<boost::thread> > subscriberThreadMap;
        
        /**
         * @brief Thread performing all the publishing on redis queues.
         */
        boost::thread* mPublisherThread;

        /**
         * @brief Queue storing all messages to be published
         */
        std::queue<std::pair<std::string, json> > mPublishingQueue;

        /**
         * @brief Mutex protecting the publishing queue from multi access
         */
        boost::mutex mPubMutex;

        /**
         * @brief condition lifted when a message is enqueued, activating the publishing thread
         * 
         */
        boost::condition_variable_any mCondition;

        /**
         * @brief putting it to false will stop the publishing thread
         */
        bool mPublisherNotStopped;

        /**
         * @brief Publishing thread method
         */
        void publishRunner();
};

#endif //PRIVATE_NETWORK_ACCESS_REDIS_H