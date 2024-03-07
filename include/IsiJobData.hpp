#ifndef ISI_JOB_DATA_H
#define ISI_JOB_DATA_H

#include <string>
#include "JSON/json.hpp"

using json = nlohmann::json;

struct IsiJobData{
    public:
        IsiJobData(json jsonData){
            rawJSON = jsonData;
            payload = rawJSON.value("payload", json::object());
            taskId = rawJSON.value("metadata", json::object()).value("taskId", 0);
            name = rawJSON.value("metadata", json::object()).value("name", "");
            missionId = rawJSON.value("metadata", json::object()).value("missionId", 0);
            type = rawJSON.value("metadata", json::object()).value("type", "");
            designatedRobot = rawJSON.value("metadata", json::object()).value("designatedRobot", "");
            isGeneric = rawJSON.value("metadata", json::object()).value("isGeneric", false);
        };

        IsiJobData(const IsiJobData &other)
        {
            rawJSON = other.rawJSON;
            payload = other.payload;
            name = other.name;
            taskId = other.taskId;
            missionId = other.missionId;
            type = other.type;
            designatedRobot = other.designatedRobot;
            isGeneric = other.isGeneric;
        }

        json rawJSON;
        json payload;
        unsigned int taskId;
        std::string name;
        std::string type;
        std::string designatedRobot;
        unsigned int missionId;
        bool isGeneric;
};

#endif // ISI_JOB_DATA_H