// BUILD :
// 1)     cmake . (on project root location)
// 2)     cd build
// 3)     make
// EXECUTE :
//      ./client [filename.drg]

#include <iostream>  // Inclut la bibliothèque d'entrée/sortie
#include <fstream>   // Inclut la bibliothèque pour la lecture/écriture de fichiers
#include <string>    // Inclut la bibliothèque pour manipuler les chaînes de caractères
#include <vector>    // Inclut la bibliothèque pour utiliser des vecteurs
#include <sstream>   // Inclut la bibliothèque pour manipuler des flux de chaînes de caractères
#include <algorithm> // Inclut la bibliothèque pour utiliser la fonction std::sort
#include <cstdlib>
#include <map>

//Librairies OPCUA
#include "open62541pp/open62541pp.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/client.h>

//Librairie personnelle pour traitement des fichiers Driller
#include "driller_frames.h"

extern std::vector<Outil> toolBank;
// CONSTANTES
const int N_ALARMES = 999;
const opcua::Logger logger;
// Define a mapping of enum values to their corresponding names
std::map<int, std::string> EN_Robot_State = {
    {0, "NC"},
    {1, "Mode_Manuel"},
    {2, "Reff"},
    {3, "Init"},
    {4, "Acces_Zone"},
    {5, "Wait"},
    {10, "Cycle_D"},
    {11, "Cycle_C"},
    {12, "Cycle_T"},
    {13, "Cycle_S"},
    {14, "Cycle_F"},
    {15, "Cycle_R"},
    {16, "Cycle_B"}
};

// AKEROS : INTEGRER DANS LES FONCTiONS

UA_NodeId vitesse_robot = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.GVL_Config.Speed"));                          // Vitesse du robot (Exprime en %)
UA_NodeId repere_tole_opcua = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.User_PRG.Robot_Cantilever.Repere_tole")); // Repere TOLE
UA_NodeId reception_mission = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.User_PRG.Trame_IN_Akeros"));              // Reception de la mission
UA_NodeId echo_fin_mission = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.User_PRG.Trame_OUT_Akeros"));              // Envoi de l'Echo | Envoi de fin de mission
UA_NodeId lancement_mission = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.User_PRG.RAZ_Trame_IN_Akeros"));          // Bit de lancement de mission

void software_intro_section()
{

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

void getCurrentDateTime(opcua::Client &client)
{
printf("Not handled yet.");
}

void etat_robot_get(opcua::Client &client)
{
    try {
        opcua::NodeId nodeID(4, "|var|UHX65A.Application.User_PRG.Robot_Cantilever.OUT.State");
        opcua::Node node = client.getNode(nodeID);
        opcua::Variant readResult = node.readValue();

        if (!readResult.isEmpty())
        {
            if (readResult.isScalar())
            {
                uint8_t valueByte;

                const UA_DataType* data_type = readResult.getDataType();
                //std::cout << "Data Type Kind: " << data_type->typeKind << std::endl;

                if (data_type->typeKind == UA_DATATYPEKIND_INT16)
                {
                    int16_t etat_enum = *static_cast<int16_t*>(readResult.data());
                    
                    std::stringstream logMessage;
                    logMessage << "État du robot: " << EN_Robot_State[etat_enum] << std::endl;
                    log(client,opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());
                    // Handle the INT16 value as needed
                }
                else
                {
                    std::cout << "Type de donnée pas pris en compte" << std::endl;
                }
            }
            else
            {
                std::cout << "Type de donnée pas scalaire." << std::endl;
            }
        }
        else
        {
            std::cout << "Error reading etat_robot" << std::endl;
        }
    } catch (const opcua::BadStatus& e) {
        // Handle OPC UA BadStatus exception
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
    }
}












void etat_alarmes_get(opcua::Client &client)
{
    try {
        std::vector<int> activeAlarms; // Store the numbers of active alarms

        for (int i = 1; i <= N_ALARMES; ++i)
        {
            std::string numero_alarme = std::to_string(i);
            std::string etat_alarme_str = "|var|UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + numero_alarme + "].Active";
            opcua::NodeId etat_alarme_node(4, etat_alarme_str);
            opcua::Node etat_alarme = client.getNode(etat_alarme_node);
            opcua::Variant readResult = etat_alarme.readValue();

            if (!readResult.isEmpty() && readResult.isScalar() && readResult.getDataType()->typeKind == UA_DATATYPEKIND_BOOLEAN)
            {
                bool isActive = *static_cast<bool *>(readResult.data());
                if (isActive)
                {
                    activeAlarms.push_back(i); // Alarm is active, store the number
                }
            }
            else
            {
                std::cout << "Error reading etat_alarme[" << numero_alarme << "]: Invalid data" << std::endl;
            }
        }

// Print the numbers of active alarms
    if (!activeAlarms.empty())
    {
        std::stringstream logMessage;
        logMessage << "Active Alarms: ";
        for (size_t i = 0; i < activeAlarms.size(); ++i)
        {
            logMessage << activeAlarms[i];
            if (i < activeAlarms.size() - 1)
            {
                logMessage << ", ";
            }
        }
        logMessage << ".";
        log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());
    }
else
{
    std::cout << "No active alarms." << std::endl;
}
    } catch (const opcua::BadStatus& e) {
        // Handle OPC UA BadStatus exception
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


void texte_alarme_get(opcua::Client &client)
{
    try {
        std::string alarm_choice;
        std::cout << "Select the alarm number: ";
        std::cin >> alarm_choice;

        std::string texte_alarme_str = "|var|UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + alarm_choice + "].Text";

        
        opcua::NodeId texte_alarme(4, texte_alarme_str);

        opcua::Variant variantValue = client.getNode(texte_alarme).readValue();

        if (!variantValue.isEmpty())
        {
            if (variantValue.isScalar())
            {
                std::cout << "Data Type Kind: " << variantValue.getDataType()->typeKind << std::endl;
                if (variantValue.getDataType()->typeKind == UA_DATATYPEKIND_STRING)
                {
                    std::string valueString = *static_cast<std::string*>(variantValue.data());
                    std::stringstream logMessage;
                    logMessage << "Texte de l'alarme : " << valueString;
                    log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());
                }
                else
                {
                    std::stringstream logMessage;
                    logMessage << "La réponse n'est sous le format String";
                    log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
                }
            }
            else
            {
                std::stringstream logMessage;
                logMessage << "Type de donnée pas scalaire.";
                log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
            }
        }
        else
        {
            std::stringstream logMessage;
            logMessage << "Aucun texte reçu.";
            log(client, opcua::LogLevel::Warning, opcua::LogCategory::Server, logMessage.str());
        }
    } catch (const opcua::BadStatus& e) {
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}





void vitesse_robot_get(opcua::Client &client)
{
    printf("VITESSE_ROBOT : Not handled yet.");
}


void position_robot_get(opcua::Client &client) {
    try {
        opcua::NodeId pos_robot(4, "|var|UHX65A.Application.User_PRG.Robot_Cantilever.OUT.Position");
        opcua::Variant variantValue = client.getNode(pos_robot).readValue();

        if (!variantValue.isEmpty()) {
            if (variantValue.getDataType()->typeKind == UA_DATATYPEKIND_EXTENSIONOBJECT) {
                //UA_ExtensionObject* eo = reinterpret_cast<UA_ExtensionObject*>(input->data);
                opcua::ExtensionObject* eo = reinterpret_cast<opcua::ExtensionObject*>(variantValue.data());
                std::cout << eo->isDecoded() << std::endl;
                
            } else {
                std::stringstream logMessage;
                logMessage << "Variant is not a UA_DATATYPEKIND_EXTENSIONOBJECT.";
                log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
            }
        } else {
            std::stringstream logMessage;
            logMessage << "Empty data for position data.";
            log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
        }
    } catch (const opcua::BadStatus& e) {
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}





void repere_tole_get(opcua::Client &client)
{
    printf("REPERE_TOLE : Not handled yet.");
}

void lancement_mission_send(opcua::Client &client)
{
    printf("LANCEMENT_MISSION : Not handled yet.");
    // TEST ENVOI COMMANDE
    /*
    UA_Variant commandValue;
    UA_Variant_init(&commandValue);

    char* commandString = "MS A D 365527 143394 060 01 0800 500"; //Exemple de trame
    UA_String myString = UA_STRING(commandString);
    UA_Variant_setScalarCopy(&commandValue, &myString, &UA_TYPES[UA_TYPES_STRING]);


    UA_StatusCode writeStatus = UA_Client_write(client, etat_robot, commandValue);
    if (writeStatus != UA_STATUSCODE_GOOD) {
        // Handle write error
    }
    */
}

int runMenu(opcua::Client &client)
{
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
        [&]
        { etat_alarmes_get(client); },
        [&]
        { texte_alarme_get(client); },
        [&]
        { vitesse_robot_get(client); },
        [&]
        { etat_robot_get(client); },
        [&]
        { position_robot_get(client); },
        [&]
        { repere_tole_get(client); },
        [&]
        { lancement_mission_send(client); }};

    while (running)
    {
        // Display the menu
        std::cout << "\n------------------------------------------------------------" << std::endl;
        std::cout << "|                       MENU PRINCIPAL                     |" << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;

        // Display menu items dynamically
        for (size_t i = 0; i < menuItems.size(); i++)
        {
            std::cout << "|    " << i + 1 << ". " << menuItems[i] << "|" << std::endl;
        }

        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Selection: ";

        std::cin >> choice;

        if (choice >= 1 && static_cast<size_t>(choice) <= menuItems.size())
        {
            // Handle the selected menu item based on the choice
            if (choice == menuItems.size())
            {
                // Quit the program
                running = false;
            }
            else
            {
                // Call the corresponding function
                menuFunctions[choice - 1]();
            }
        }
        else
        {
            std::cout << "Invalid choice. Please select a valid task." << std::endl;
        }
    }

    return 0; // Return 0 to indicate successful exit
}

int main(int argc, char *argv[])
{

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

    //UA_Client *client = UA_Client_new();
    //UA_ClientConfig *clientConfig = UA_Client_getConfig(client);
    //UA_ClientConfig_setDefault(clientConfig);
    //clientConfig->logger = logger;


    opcua::Client client;
client.setLogger([](auto level, auto category, auto msg) {
    std::string logLevelColor, logCategoryColor, resetColor;
    
    // Set colors based on the log level and category
    switch (level) {
        case opcua::LogLevel::Trace:
            logLevelColor = "\x1b[34m"; // Blue
            break;
        case opcua::LogLevel::Debug:
            logLevelColor = "\x1b[32m"; // Green
            break;
        case opcua::LogLevel::Info:
            logLevelColor = "\x1b[36m"; // Cyan
            break;
        case opcua::LogLevel::Warning:
            logLevelColor = "\x1b[33m"; // Yellow
            break;
        case opcua::LogLevel::Error:
            logLevelColor = "\x1b[31m"; // Red
            break;
        case opcua::LogLevel::Fatal:
            logLevelColor = "\x1b[35m"; // Magenta
            break;
        default:
            break;
    }

    switch (category) {
        case opcua::LogCategory::Network:
            logCategoryColor = "\x1b[34m"; // Blue
            break;
        case opcua::LogCategory::SecureChannel:
            logCategoryColor = "\x1b[32m"; // Green
            break;
        case opcua::LogCategory::Session:
            logCategoryColor = "\x1b[36m"; // Cyan
            break;
        case opcua::LogCategory::Server:
            logCategoryColor = "\x1b[33m"; // Yellow
            break;
        case opcua::LogCategory::Client:
            logCategoryColor = "\x1b[31m"; // Red
            break;
        case opcua::LogCategory::Userland:
            logCategoryColor = "\x1b[35m"; // Magenta
            break;
        case opcua::LogCategory::SecurityPolicy:
            logCategoryColor = "\x1b[35m"; // Magenta
            break;
        default:
            break;
    }

    resetColor = "\x1b[0m"; // Reset to default color

    std::cout << logLevelColor << "[" << opcua::getLogLevelName(level) << "] "
              << logCategoryColor << "[" << opcua::getLogCategoryName(category) << "] "
              << resetColor << msg << std::endl;
});

    std::cout << "Client Object Created!" << std::endl;
    client.connect("opc.tcp://192.168.100.14:4840");

    opcua::Node node = client.getNode(opcua::VariableId::Server_ServerStatus_CurrentTime);
    const auto dt = node.readValueScalar<opcua::DateTime>();

    std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;



    int menu_exit_code = runMenu(client); // Lancement du menu DRILLER

    //------------------------------------------- END TEST ENV FOR FUNCTIONS -----------------------
/*
    switch (menu_exit_code)
    {
    case 0:
        UA_LOG_INFO(&logger, UA_LOGCATEGORY_SERVER, "Sortie du programme | CODE : %d", menu_exit_code);
        break;
    default:
        UA_LOG_ERROR(&logger, UA_LOGCATEGORY_SERVER, "Erreur en sortie du programme | CODE : %d", menu_exit_code);
        break;
    }

    return menu_exit_code;

    */
}