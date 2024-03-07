// STD
#include <cstdlib>
#include <iostream>

#include "JSON/json.hpp"

#include "IsiConnect.hpp"
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>
#include "IsiStats.hpp"


class Test : public IsiConnect<Test>
{
    public:
        Test(std::string robotType, std::string uuid = "") : IsiConnect<Test>(robotType, uuid) {}
        
        bool isAvailable() {return true;}
        
        virtual json emergencyStop(json data, std::string jobId) override 
        {
            json status;
            BOOST_LOG_TRIVIAL(debug) << "emergencyStop called";
            return status;
        } 

        json callback(json data, std::string jobId, IsiStats& stats)
        {
            json status;
            BOOST_LOG_TRIVIAL(debug) << "callback called";
            BOOST_LOG_TRIVIAL(debug) << "Toto is " << data["toto"].get<unsigned int>();

            /*status["status"] = "failed";
            status["failedReason"] = "because I want";
            status["stacktrace"] = getBacktrace();*/
            stats.electricalConsumption = 150;
            stats.consumedRawMaterials.push_back(RawMaterial("toto", 10, "kg"));
            stats.consumedRawMaterials.push_back(RawMaterial("titi", 20, "L"));


            status["test"] = "yay";
            status["more"] = "we did it!";

            std::string alarmId = publishAlarm("Alarme de test", "C'est juste pour tester tavu");

            // Test loop
            int i = 0;
            while(i < 10)
            {
                updateProgress(jobId, 10 * i++, data);
                logTask(jobId, "test log : " + std::to_string(i));
                sleep(1);
            }

            ackAlarm(alarmId);

            return status;
        }
};



int main()
{
    Test object("testRobot");
    object.init();
    object.addJob("loadingTask", boost::bind(&Test::callback, &object, _1, _2, _3));


    int i = 0;
    
    // Test loop
    while(true)
    {
        sleep(1);
        i++;
    }
    return EXIT_SUCCESS;
}

