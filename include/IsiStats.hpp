/** 
 * @file IsiStats.hpp
 * @brief Class that contains all the statistics of a production step
 * 
 * @author 
 * @version 1.0 
 * @date 2020-12-01 
 */

#ifndef ISI_STATS_H
#define ISI_STATS_H

#include <vector>
#include <string>
#include "JSON/json.hpp"

using json = nlohmann::json;

/**
 * @brief Class describing a raw material entry
 * 
 */
struct RawMaterial {
    public:
        RawMaterial(std::string name = "", unsigned int quantity = 0, std::string unit = "") : name(name), quantity(quantity), unit(unit) {}
        std::string name;
        unsigned int quantity;
        std::string unit;
};


/**
 * @brief Function to convert a rawMaterial object to a json object
 * 
 * @param j json object
 * @param rm rawMaterial object
 */
void to_json(json& j, const RawMaterial& rm);

/**
 * @brief Class that contains all the statistics of a production step
 * 
 */
struct IsiStats {
    public:
        IsiStats() {
            this->status = "";
            this->taskId = 0;
            this->startTime = 0;
            this->elapsedTime = 0;
            this->productNb = 0;
            this->failedProductNb = 0;
            this->electricalConsumption = 0;
        }

        std::string status;
        unsigned int startTime;
        unsigned int taskId;
        unsigned int elapsedTime;
        unsigned int productNb;
        unsigned int failedProductNb;
        unsigned int electricalConsumption;
        std::vector<RawMaterial> consumedRawMaterials;
};

/**
 * @brief Function to convert a IsiStats object to a json object
 * 
 * @param j json object
 * @param isi IsiStats object
 */
void to_json(json& j, const IsiStats& isi);

#endif