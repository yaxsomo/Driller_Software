/**
 * @file bullmq_lua.hpp
 * @author your name (alexandre.jumeline@akeros.com)
 * @brief File containing all LUA related calls
 * @version 0.1
 * @date 2023-01-12
 * 
 * @copyright Copyright (c) Akeros 2023
 * 
 */

#ifndef BULL_MQ_LUA_H
#define BULL_MQ_LUA_H

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/log/trivial.hpp>
#include <openssl/sha.h>

#define BULLMQ_LUA_ADD_JOB "bullmq_LUA_unified/addJob.lua"
#define BULLMQ_LUA_MOVE_STALLED_JOBS_TO_WAIT "bullmq_LUA_unified/moveStalledJobsToWait.lua"
#define BULLMQ_LUA_MOVE_TO_ACTIVE "bullmq_LUA_unified/moveToActive.lua"
#define BULLMQ_LUA_MOVE_FROM_ACTIVE_TO_A_FINISHED_STATUS "bullmq_LUA_unified/moveToFinished.lua"
#define BULLMQ_LUA_UPDATE_PROGRESS "bullmq_LUA_unified/updateProgress.lua"
#define BULLMQ_LUA_EXTEND_LOCK "bullmq_LUA_unified/extendLock.lua"

namespace lua_utils
{
    /**
     * @brief Get the Lua File As Str object
     * This is used to load the lua scripts used by BullMq in Redis.
     * 
     * @param file File to be loaded in a string
     * @return std::string string containing the lua script
     */
    static std::string getLuaFileAsStr(std::string file)
    {
        try
        {
            if (file.substr(file.find_last_of(".") + 1) == "lua")
            {
                std::ifstream t(file);
                if (t.fail()){
                    throw std::invalid_argument(file + " not found !");
                }
                std::stringstream buffer;
                buffer << t.rdbuf();
                return buffer.str();
            } else {
                throw std::invalid_argument(file);
            }
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error) << "Lua file loader error : " << e.what() << '\n';
            return "";
        }
    }

    /**
     * @brief Get the Lua Script SHA1 digest in order to eval a script from redis' cache
     * 
     * @param script To thet the SHA1 from
     * @return std::string String containing the SHA1
     */
    static std::string getLuaScriptSHA1(std::string script)
    {
        char outputBuffer[SHA_DIGEST_LENGTH*2];

        // Compute sha1 digest of the script
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1((const unsigned char*)script.c_str(), script.length(), hash);

        // Convert it to a string
        for(int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        {
            sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
        }
        return std::string(outputBuffer);
    }
}

#endif //BULL_MQ_LUA_H