/**
 * @file isiAlarm.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef ISI_ALARM_HPP
#define ISI_ALARM_HPP

#include <string>
#include <memory>
#include "JSON/json.hpp"

#include "DAO/privateNetworkAccess.hpp"

#define ALARMS_QUEUE_NAME "ISI.ALARM"

using json = nlohmann::json;

/**
 * @brief Class sending alarms to the Isi Core
 * 
 */
class IsiAlarm {
    public:
        /**
         * @brief Construct a new Isi Alarm object
         * 
         * @param isiName Name of the IsiConnect object
         * @param access Internal network accessor pointer
         */
        IsiAlarm(std::string isiName, std::shared_ptr<PrivateNetworkAccess> access);

        /**
         * @brief Method sending an alarm the the isi core
         * 
         * @param alarm Data sent with the alarm
         * Alarm data must be a json object with the following fields :
         * json alarm = 
         *  {
         *       {"name", name},
         *       {"description", description},
         *       {"uuid", uuid},
         *       {"created", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
         *       {"active", true}
         *   };
         */
        std::string publishNewAlarm(const std::string &name, const std::string &description, const std::string &timestamp="");

        /**
         * @brief Method acknowledging the alarm with specified uuid
         * 
         * @param uuid Uuid of the alarm to acknowledge
         * Alarm data must be a json object with the following fields :
         * json alarm =
         * {
         *      {"uuid", uuid},
         *     {"active", false}
         * };
         */
        void ackAlarm(const std::string &uuid);

    private:
        /**
         * @brief Name of the IsiConnect object instanciating the alarm object.
         */
        std::string mIsiName;

        /**
         * @brief Pointer to the internal network accessor.
         */
        std::shared_ptr<PrivateNetworkAccess> mPrivateNetAccessor;
};

#endif //ISI_ALARM_HPP