//BUILD : 
//1)     cmake . (on project root location)
//2)     cd build
//3)     make
//EXECUTE :
//     ./client [filename.drg]

#include <iostream>       // Inclut la bibliothèque d'entrée/sortie
#include <fstream>        // Inclut la bibliothèque pour la lecture/écriture de fichiers
#include <string>         // Inclut la bibliothèque pour manipuler les chaînes de caractères
#include <vector>         // Inclut la bibliothèque pour utiliser des vecteurs
#include <sstream>        // Inclut la bibliothèque pour manipuler des flux de chaînes de caractères
#include <algorithm>      // Inclut la bibliothèque pour utiliser la fonction std::sort
#include <cstdlib>
#include "open62541pp/open62541pp.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/client.h>



#include "driller_frames.h"

extern std::vector<Outil> toolBank;
//VARIABLES AKEROS
const int N_ALARMES = 50;
std::string numero_alarme = "1"; // Replace with the actual alarm number as a string
std::string etat_alarme_str = "UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + numero_alarme + "].Active";
std::string texte_alarme_str = "UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + numero_alarme + "].Text";

UA_NodeId etat_alarme = UA_NODEID_STRING(4, const_cast<char*>(etat_alarme_str.c_str())); //Etat de l'alarme (1 = Active | 0 = Inactive)
UA_NodeId texte_alarme = UA_NODEID_STRING(4, const_cast<char*>(texte_alarme_str.c_str())); // Texte de l'alarme
UA_NodeId vitesse_robot = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.GVL_Config.Speed")); // Vitesse du robot (Exprime en %)
UA_NodeId etat_robot = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Robot_Cantilever.OUT.State")); // Etat actuel du robot
UA_NodeId position_robot = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Robot_Cantilever.OUT.Position")); // Position actuelle du robot (Repere ROBOT)
UA_NodeId repere_tole_opcua = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Robot_Cantilever.Repere_tole")); // Repere TOLE
UA_NodeId reception_mission = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Trame_IN_Akeros")); // Reception de la mission
UA_NodeId echo_fin_mission = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Trame_OUT_Akeros")); // Envoi de l'Echo | Envoi de fin de mission
UA_NodeId lancement_mission = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.RAZ_Trame_IN_Akeros")); // Bit de lancement de mission


void software_intro_section(){

    std::string asciiArt = R"(
██████╗ ██████╗ ██╗██╗     ██╗     ███████╗██████╗     ███████╗ ██████╗ ███████╗████████╗██╗    ██╗ █████╗ ██████╗ ███████╗
██╔══██╗██╔══██╗██║██║     ██║     ██╔════╝██╔══██╗    ██╔════╝██╔═══██╗██╔════╝╚══██╔══╝██║    ██║██╔══██╗██╔══██╗██╔════╝
██║  ██║██████╔╝██║██║     ██║     █████╗  ██████╔╝    ███████╗██║   ██║█████╗     ██║   ██║ █╗ ██║███████║██████╔╝█████╗  
██║  ██║██╔══██╗██║██║     ██║     ██╔══╝  ██╔══██╗    ╚════██║██║   ██║██╔══╝     ██║   ██║███╗██║██╔══██║██╔══██╗██╔══╝  
██████╔╝██║  ██║██║███████╗███████╗███████╗██║  ██║    ███████║╚██████╔╝██║        ██║   ╚███╔███╔╝██║  ██║██║  ██║███████╗
╚═════╝ ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝    ╚══════╝ ╚═════╝ ╚═╝        ╚═╝    ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝                                                                                   
)";
    std::cout << asciiArt << std::endl;

}


void getCurrentDateTime(UA_Client *client, UA_NodeId node) {
    UA_DataValue dataValue;
    UA_StatusCode readStatus = UA_Client_readValueAttribute(client, node, &dataValue.value);

    if (readStatus == UA_STATUSCODE_GOOD) {
        // Successfully read the value. You can access the value using dataValue.value.
        if (dataValue.hasValue) {
            UA_Variant variantValue = dataValue.value;
            if (variantValue.type == &UA_TYPES[UA_TYPES_DATETIME]) {
                UA_DateTime dateTimeValue = *(UA_DateTime*)variantValue.data;

                UA_DateTimeStruct dtStruct = UA_DateTime_toStruct(dateTimeValue);

                printf("Date and Time: %d-%02d-%02d %02d:%02d:%02d\n",
                       dtStruct.year, dtStruct.month, dtStruct.day,
                       dtStruct.hour, dtStruct.min, dtStruct.sec);
                // TODO : Clean up the allocated memory
            } else {
                // Handle data type mismatch or other cases as needed.
                printf("The variable is not of DateTime type.\n");
            }
        }
    } else {
        // Handle read error.
        printf("Error reading %.*s: %s\n ", node.namespaceIndex, node.identifier.string.data, UA_StatusCode_name(readStatus));
    }
}



void etat_robot_get(UA_Client *client, UA_NodeId node) {
    UA_DataValue dataValue;
    UA_StatusCode readStatus = UA_Client_readValueAttribute(client, node, &dataValue.value);

    if (readStatus == UA_STATUSCODE_GOOD) {
        // Successfully read the value. You can access the value using dataValue.value.
        if (dataValue.hasValue) {
            UA_Variant variantValue = dataValue.value;
            if (variantValue.type == &UA_TYPES[UA_TYPES_STRING]) {
                UA_String valueString = *(UA_String*)variantValue.data;
                printf("Value of %.*s: %.*s\n", node.namespaceIndex, node.identifier.string.data,
                       (int)valueString.length, valueString.data);
            } else {
                // Handle data type mismatch or other cases as needed.
            }
        }
    } else {
        // Handle read error.
        printf("Error reading %.*s: %s\n ", node.namespaceIndex, node.identifier.string.data, UA_StatusCode_name(readStatus));
    }
}


void etat_alarmes_get(UA_Client *client) {
    std::vector<int> activeAlarms; // Store the numbers of active alarms

    for (int i = 1; i <= N_ALARMES; ++i) {
        // Update numero_alarme
        numero_alarme = std::to_string(i);

        // Recreate etat_alarme_str with the updated numero_alarme
        etat_alarme_str = "UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + numero_alarme + "].Active";
        etat_alarme = UA_NODEID_STRING(4, const_cast<char*>(etat_alarme_str.c_str()));

        UA_Variant variantValue;
        UA_StatusCode readStatus = UA_Client_readValueAttribute(client, etat_alarme, &variantValue);
        if (readStatus == UA_STATUSCODE_GOOD) {
            if (variantValue.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                UA_Boolean valueBoolean = *(UA_Boolean*)variantValue.data;
                if (valueBoolean) {
                    activeAlarms.push_back(i); // Alarm is active, store the number
                }
            } else {
                // Handle data type mismatch or other cases as needed.
            }
        } else {
            // Handle read error.
            std::cout << "Error reading etat_alarme: " << UA_StatusCode_name(readStatus) << std::endl;
        }
    }

    // Print the numbers of active alarms
    if (!activeAlarms.empty()) {
        std::cout << "Active alarms: ";
        for (int alarmNumber : activeAlarms) {
            std::cout << alarmNumber << ", ";
        }
        std::cout << "." << std::endl;
    } else {
        std::cout << "No active alarms." << std::endl;
    }
}

void texte_alarme_get(UA_Client *client){
    printf("TEXTE_ALARME : Not handled yet.");
}

void vitesse_robot_get(UA_Client *client){
    printf("VITESSE_ROBOT : Not handled yet.");
}
void position_robot_get(UA_Client *client){
    printf("POSISION_ROBOT : Not handled yet.");
}

void repere_tole_get(UA_Client *client){
    printf("REPERE_TOLE : Not handled yet.");
}

void lancement_mission_send(UA_Client *client){
    printf("LANCEMENT_MISSION : Not handled yet.");
    //TEST ENVOI COMMANDE
    /*
    UA_Variant commandValue;
    UA_Variant_init(&commandValue);
    
    char* commandString = "YourStringCommandHere";
    UA_String myString = UA_STRING(commandString);
    UA_Variant_setScalarCopy(&commandValue, &myString, &UA_TYPES[UA_TYPES_STRING]);


    UA_StatusCode writeStatus = UA_Client_write(client, etat_robot, commandValue);
    if (writeStatus != UA_STATUSCODE_GOOD) {
        // Handle write error
    }
    */
}


int runMenu(UA_Client *client) {
    bool running = true;
    int choice = 0;

    // Create a vector to store menu items
    std::vector<std::string> menuItems = {
        "État des alarmes                                   ",
        "Texte de l'alarme                                  ",
        "Vitesse du robot (en %)                            ",
        "État du robot                                      ",
        "Position du robot                                  ",
        "Repere Tôle                                        ",
        "Lancement de mission (Une trame)                   ",
        "QUIT PROGRAM                                       " // Doit imperativement rester en derniere position
    };

    // Create a vector of function pointers
    std::vector<std::function<void()>> menuFunctions = {
        [&] {etat_alarmes_get(client); },
        [&] { texte_alarme_get(client); },
        [&] { vitesse_robot_get(client); },
        [&] { etat_robot_get(client, etat_robot); },
        [&] { position_robot_get(client); },
        [&] { repere_tole_get(client); },
        [&] { lancement_mission_send(client); }
    };

    while (running) {
        // Display the menu
        std::cout << "\n------------------------------------------------------------" << std::endl;
        std::cout << "|                       MENU PRINCIPAL                     |" << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;

        // Display menu items dynamically
        for (size_t i = 0; i < menuItems.size(); i++) {
            std::cout << "|    "<< i + 1 << ". " << menuItems[i] << "|" <<std::endl;
        }

        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Selection: ";

        std::cin >> choice;

        if (choice >= 1 && static_cast<size_t>(choice) <= menuItems.size()) {
            // Handle the selected menu item based on the choice
            if (choice == menuItems.size()) {
                // Quit the program
                running = false;
            } else {
                // Call the corresponding function
                menuFunctions[choice - 1]();
            }
        } else {
            std::cout << "Invalid choice. Please select a valid task." << std::endl;
        }
    }

    return 0; // Return 0 to indicate successful exit
}





int main(int argc, char* argv[]) {

    software_intro_section();

/*
    // ------------------------------ OPCUA CLIENT AND LOGGER SECTION --------------------------------------
    // Set up the logger to print log messages to the console
    const UA_Logger logger = UA_Log_Stdout_;

    // Create a client instance
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *clientConfig = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(clientConfig);
    clientConfig->logger = logger;

    // Connect to the server
    const char *serverUrl = "opc.tcp://192.168.100.14:4840";
    UA_StatusCode connectStatus = UA_Client_connect(client, serverUrl);

    if (connectStatus != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(&logger, UA_LOGCATEGORY_SERVER, "Probleme de connexion à la PLC");
        // Handle connection error
        UA_Client_delete(client); // Clean up the client if the connection fails
        return -1; // Or some other error handling
    }

    
    // Specify the NodeId for the current time variable (Node 2258)
    opcua::NodeId currentTimeNodeId(opcua::VariableId::Server_ServerStatus_CurrentTime);
    getCurrentDateTime(client, currentTimeNodeId);

    // ------------------------------ END OPCUA CLIENT AND LOGGER SECTION --------------------------------------

*/


//------------------------------------------- TEST ENV FOR FUNCTIONS -----------------------

    const UA_Logger logger = UA_Log_Stdout_;
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *clientConfig = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(clientConfig);
    clientConfig->logger = logger;
    int menu_exit_code = runMenu(client); // Lancement du menu DRILLER

//------------------------------------------- END TEST ENV FOR FUNCTIONS -----------------------
    



   switch(menu_exit_code){
    case 0:
        UA_LOG_INFO(&logger, UA_LOGCATEGORY_SERVER, "Sortie du programme | CODE : %d", menu_exit_code);
        break;
    default:
        UA_LOG_ERROR(&logger, UA_LOGCATEGORY_SERVER, "Erreur en sortie du programme | CODE : %d", menu_exit_code);
        break;
   }
   

   return menu_exit_code;

    

    
}