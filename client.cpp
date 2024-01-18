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
#include <cstring> // for strlen

// Librairies OPCUA
#include "open62541pp/open62541pp.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/client.h>

#include <chrono>
#include <thread>

// Librairie personnelle pour traitement des fichiers Driller
#include "driller_frames.h"

extern std::vector<Outil> toolBank;
// CONSTANTES
const int N_ALARMES = 999;
const opcua::Logger logger;
// Mapping des etats du robot
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
    {16, "Cycle_B"}};

std::map<int, std::string> OPERATIONS_TYPES = {
    {1, "Perçage"},
    {2, "Taraudage"},
    {3, "Taraudage"},
    {4, "Fraisurage"},
    {5, "Lamage"}};

std::map<std::string, std::string> TRAMES_OPERATIONNELLES = {
    {"ARRET_CYCLES", "MS O ABORD"},   // Commande de stop de l'ensemble des cycles auto
    {"ACK_AL_DEF", "MS O ACK_ALARM"}, // Aquitter les alarmes et defauts
    {"INIT_ROBOT", "MS O INIT"},      // Lancement du cycle d'initialisation du robot
    {"REF_ROBOT", "MS O REFF"},       // Lancement du referencement robot
};

std::map<std::string, std::string> COMMANDES_MODE_MANUEL = {
    {"POSITION_MAINTENANCE", "MS M M"}, // Commande de stop de l'ensemble des cycles auto
    {"ROTATION_OUTIL", "MS M R"},       // Aquitter les alarmes et defauts
    {"EXTRACTION_OUTIL", "MS M E"},     // Lancement du cycle d'initialisation du robot
    {"CHANGEMENT_OUTIL", "MS M S"},     // Lancement du referencement robot
};

std::map<std::string, std::string> COMMANDES_ROBOT_MANUEL = {
    {"X", "MS M X"}, // Commande de stop de l'ensemble des cycles auto
    {"Y", "MS M Y"}, // Aquitter les alarmes et defauts
    {"Z", "MS M Z"}, // Lancement du cycle d'initialisation du robot
};

std::map<int, std::string> OPERATIONAL_MODE = {
    {0, "GROUPS"},
    {1, "SECTIONS"}};

// Structure des coordonnees du robot
struct St_Coordonee
{
    double X;
    double Y;
    double Z;
};

int currentOperationalMode = 0; // Default operational mode

int16_t current_robot_state = 0;
// Global variable to signal abort
bool abortMission = false;

// AKEROS : INTEGRER DANS LES FONCTiONS
UA_NodeId vitesse_robot = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.GVL_Config.Speed"));                          // Vitesse du robot (Exprime en %)
UA_NodeId repere_tole_opcua = UA_NODEID_STRING(4, const_cast<char *>("UHX65A.Application.User_PRG.Robot_Cantilever.Repere_tole")); // Repere TOLE

void abortMissionSignal()
{
    abortMission = true;
}

// Example function using a keyboard shortcut (you can replace it with your actual implementation)
void checkForKeyboardShortcut()
{
    // Check if a specific keyboard shortcut is pressed
    // if (/* check keyboard shortcut condition */)
    //{
    //    abortMissionSignal();
    //}
}

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

int16_t etat_robot_get(opcua::Client &client)
{
    try
    {
        opcua::NodeId nodeID(4, "|var|UHX65A.Application.User_PRG.Robot_Cantilever.OUT.State");
        opcua::Node node = client.getNode(nodeID);
        opcua::Variant readResult = node.readValue();

        if (!readResult.isEmpty())
        {
            if (readResult.isScalar())
            {
                const UA_DataType *data_type = readResult.getDataType();

                if (data_type->typeKind == UA_DATATYPEKIND_INT16)
                {
                    // Store the current robot state
                    current_robot_state = *static_cast<int16_t *>(readResult.data());

                    // std::stringstream logMessage;
                    // logMessage << "État du robot: " << EN_Robot_State[current_robot_state] << std::endl;
                    // log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());

                    // Return the current robot state
                    return current_robot_state;
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
    }
    catch (const opcua::BadStatus &e)
    {
        // Handle OPC UA BadStatus exception
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // Return a default value in case of an error
    return -1;
}

void etat_alarmes_get(opcua::Client &client)
{
    try
    {
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
    }
    catch (const opcua::BadStatus &e)
    {
        // Handle OPC UA BadStatus exception
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void texte_alarme_get(opcua::Client &client)
{
    try
    {
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
                // std::cout << "Data Type Kind: " << variantValue.getDataType()->typeKind << std::endl;
                if (variantValue.getDataType()->typeKind == UA_DATATYPEKIND_STRING)
                {
                    // std::cout << "Value is string" << std::endl;
                    if (variantValue.isEmpty())
                    {
                        std::cout << "The Trame_OUT is empty" << std::endl;
                    }

                    opcua::String valueString = *static_cast<opcua::String *>(variantValue.data());
                    std::string_view valueString_view = *static_cast<std::string_view *>(variantValue.data());

                    // std::cout << "Value converted to string" << std::endl;

                    // std::cout << "Value String OPCUA: " << valueString << std::endl;
                    // std::cout << "Value String View: " << valueString_view << std::endl;

                    std::stringstream logMessage;
                    logMessage << "Texte alarme : " << valueString_view;
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
    }
    catch (const opcua::BadStatus &e)
    {
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    }
    catch (const std::bad_alloc &e)
    {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void trame_out_get(opcua::Client &client)
{
    try
    {

        opcua::NodeId trame_out(4, "|var|UHX65A.Application.User_PRG.Trame_OUT_Akeros");
        opcua::Variant variantValue = client.getNode(trame_out).readValue();

        if (!variantValue.isEmpty())
        {
            if (variantValue.isScalar())
            {
                // std::cout << "Value is scalar" << std::endl;
                // std::cout << "Data Type Kind: " << variantValue.getDataType()->typeKind << std::endl;

                if (variantValue.getDataType()->typeKind == UA_DATATYPEKIND_STRING)
                {
                    // std::cout << "Value is string" << std::endl;
                    if (variantValue.isEmpty())
                    {
                        std::cout << "The Trame_OUT is empty" << std::endl;
                    }

                    opcua::String valueString = *static_cast<opcua::String *>(variantValue.data());
                    std::string_view valueString_view = *static_cast<std::string_view *>(variantValue.data());

                    // std::cout << "Value converted to string" << std::endl;

                    // std::cout << "Value String OPCUA: " << valueString << std::endl;
                    // std::cout << "Value String View: " << valueString_view << std::endl;

                    std::stringstream logMessage;
                    logMessage << "TRAME_OUT : " << valueString_view;
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
            logMessage << "Pas de trame presente";
            log(client, opcua::LogLevel::Warning, opcua::LogCategory::Server, logMessage.str());
        }
    }
    catch (const opcua::BadStatus &e)
    {
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    }
    catch (const std::bad_alloc &e)
    {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void position_robot_get(opcua::Client &client)
{
    try
    {
        opcua::NodeId pos_robot(4, "|var|UHX65A.Application.User_PRG.Robot_Cantilever.OUT.Position");
        opcua::Variant variantValue = client.getNode(pos_robot).readValue();

        if (!variantValue.isEmpty())
        {
            if (variantValue.getDataType()->typeKind == UA_DATATYPEKIND_EXTENSIONOBJECT)
            {
                opcua::ExtensionObject *eo = reinterpret_cast<opcua::ExtensionObject *>(variantValue.data());
                // std::cout << "Data is Extension Obj" << std::endl;
                if (eo->isEncoded())
                {
                    // std::cout << "Extension Obj Encoded" << std::endl;
                    if (eo->handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING)
                    {
                        // std::cout << "dataType is Bytestring" << std::endl;

                        std::optional<opcua::ByteString> optionalEncodedBody = eo->getEncodedBody();
                        if (optionalEncodedBody && !optionalEncodedBody->empty())
                        {
                            opcua::ByteString &encodedBody = *optionalEncodedBody;
                            const void *rawData = encodedBody.get().data(); // Get raw binary data
                            size_t dataSize = encodedBody.get().size();     // Get the size of the binary data

                            // Print the raw data
                            // std::cout << "Raw Data: ";
                            // for (size_t i = 0; i < dataSize; ++i)
                            //{
                            //    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(reinterpret_cast<const uint8_t *>(rawData)[i]) << " ";
                            //}
                            // std::cout << std::dec << std::endl;

                            // std::cout << "Size of encoded data: " << dataSize << " bytes" << std::endl;
                            // std::cout << "Size of St_Coordonee structure: " << sizeof(St_Coordonee) << " bytes" << std::endl;

                            // Assuming the encoded data is a sequence of three double values
                            const float *encodedValues = static_cast<const float *>(rawData);

                            St_Coordonee decodedStructure;
                            decodedStructure.X = encodedValues[0];
                            decodedStructure.Y = encodedValues[1];
                            decodedStructure.Z = encodedValues[2];

                            std::stringstream logMessage;

                            logMessage << "Position Robot:"
                                       << " X=" << decodedStructure.X << " |"
                                       << " Y=" << decodedStructure.Y << " |"
                                       << " Z=" << decodedStructure.Z << std::endl;
                            log(client, opcua::LogLevel::Info, opcua::LogCategory::Server, logMessage.str());
                        }
                        else
                        {
                            std::cerr << "Failed to get encoded body or encoded body is empty." << std::endl;
                        }

                        // TODO SOMETHING HERE
                    }
                    else
                    {
                        std::stringstream logMessage;
                        logMessage << "Invalid data type for position data.";
                        log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
                    }
                }
                else
                {
                    std::stringstream logMessage;
                    logMessage << "Failed to decode ExtensionObject.";
                    log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
                }
            }
            else
            {
                std::stringstream logMessage;
                logMessage << "Variant is not a UA_DATATYPEKIND_EXTENSIONOBJECT.";
                log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
            }
        }
        else
        {
            std::stringstream logMessage;
            logMessage << "Empty data for position data.";
            log(client, opcua::LogLevel::Error, opcua::LogCategory::Server, logMessage.str());
        }
    }
    catch (const opcua::BadStatus &e)
    {
        std::cerr << "OPC UA BadStatus error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void mission_lancement(opcua::Client &client, const std::vector<std::string> &trames)
{
    opcua::NodeId trame_in(4, "|var|UHX65A.Application.User_PRG.Trame_IN_Akeros");
    opcua::NodeId mission_go(4, "|var|UHX65A.Application.User_PRG.RAZ_Trame_IN_Akeros");
    int mission_counter = 0;

    for (const std::string &trame : trames)
    {
        std::stringstream logMessage;
        std::cout << "------------------------------------------------------------" << std::endl;
        // Check for the abort signal
        if (abortMission)
        {
            logMessage << "Ensemble des missions anullés!" << std::endl;
            log(client, opcua::LogLevel::Warning, opcua::LogCategory::SecureChannel, logMessage.str());
            break;
        }
        // Assuming you want to use trame as the input for each iteration
        opcua::String trameInputString(trame);

        opcua::Variant trameVariant;
        trameVariant.setScalarCopy(trameInputString);

        // Write the trame to trame_in Node
        client.getNode(trame_in).writeValueScalar(trameInputString);
        logMessage << "Trame envoyée correctement!" << std::endl;
        log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());

        // Set mission_go (Bit)
        bool missionGoValue = false; // Set this value based on your logic (0 or 1)
        opcua::Variant missionGoVariant = opcua::Variant::fromScalar(missionGoValue);
        client.getNode(mission_go).writeValue(missionGoVariant);

        std::cout << "MISSION N." << mission_counter << ": " << trame << std::endl;

        // std::stringstream logMessage;
        logMessage << "État du robot: " << EN_Robot_State[current_robot_state] << std::endl;
        log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());

        // Wait until the robot finishes its mission (state becomes "Wait")
        while (true)
        {
            // Check for the abort signal
            if (abortMission)
            {
                logMessage << "Mission Anullée!" << std::endl;
                log(client, opcua::LogLevel::Warning, opcua::LogCategory::SecureChannel, logMessage.str());
                break;
            }

            // Get the current robot state
            etat_robot_get(client);

            // Add a delay between state checks (adjust as needed)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // Check if the robot is in the "Wait" state (state 5)
            if (current_robot_state == 5)
            {
                std::cout << "MISSION N." << mission_counter << ": TERMINÉ" << std::endl;
                break;
            }
        }

        trame_out_get(client);

        // Increment the mission counter
        mission_counter++;

        // Add any necessary delay or logic between commands
    }
}

void mission_lancement_une_trame(opcua::Client &client)
{
    opcua::NodeId trame_in(4, "|var|UHX65A.Application.User_PRG.Trame_IN_Akeros");
    opcua::NodeId mission_go(4, "|var|UHX65A.Application.User_PRG.RAZ_Trame_IN_Akeros");
    int mission_counter = 0;

    std::string trame = "MS A D 300000 100000 060 00 0800 500";

    std::stringstream logMessage;
    std::cout << "------------------------------------------------------------" << std::endl;
    // Assuming you want to use trame as the input for each iteration
    opcua::String trameInputString(trame);

    opcua::Variant trameVariant;
    trameVariant.setScalarCopy(trameInputString);

    // Write the trame to trame_in Node
    client.getNode(trame_in).writeValueScalar(trameInputString);
    logMessage << "Trame envoyée correctement!" << std::endl;
    log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());

    // Set mission_go (Bit)
    bool missionGoValue = false; // Set this value based on your logic (0 or 1)
    opcua::Variant missionGoVariant = opcua::Variant::fromScalar(missionGoValue);
    client.getNode(mission_go).writeValue(missionGoVariant);

    std::cout << "MISSION N." << mission_counter << ": " << trame << std::endl;

    // std::stringstream logMessage;
    logMessage << "État du robot: " << EN_Robot_State[current_robot_state] << std::endl;
    log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());

    // Wait until the robot finishes its mission (state becomes "Wait")
    while (true)
    {

        // Get the current robot state
        etat_robot_get(client);

        // Add a delay between state checks (adjust as needed)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Check if the robot is in the "Wait" state (state 5)
        if (current_robot_state == 5)
        {
            std::cout << "MISSION N." << mission_counter << ": TERMINÉ" << std::endl;
            break;
        }
    }

    trame_out_get(client);
    position_robot_get(client);
}

// Function to write the command to the robot node
void trames_ope_write(opcua::Client &client, const std::string &commandKey)
{
    std::stringstream logMessage;
    // Check if the key exists in the map
    // Check if the key exists in the map
    auto it = TRAMES_OPERATIONNELLES.find(commandKey);
    if (it != TRAMES_OPERATIONNELLES.end())
    {
        // Get the corresponding command
        std::string command = it->second;

        // Write the command to the robot node
        opcua::NodeId robotNode(4, "|var|UHX65A.Application.User_PRG.Trame_IN_Akeros");

        opcua::String trame_ope(command);

        opcua::Variant trame_ope_var;
        trame_ope_var.setScalarCopy(trame_ope);

        // Write the trame to trame_in Node
        client.getNode(robotNode).writeValueScalar(trame_ope);

        logMessage << "Commande écrite correctement: " << command << std::endl;
        log(client, opcua::LogLevel::Trace, opcua::LogCategory::Server, logMessage.str());
    }
    else
    {
        std::cerr << "Error: Command key not found in the map." << std::endl;
    }
}

// Function to write manual command to the robot node
void commandes_manuelles_write(opcua::Client &client, const std::string &commandKey, const std::string &value = "")
{
    std::stringstream logMessage;

    // Check if the key exists in the map
    auto it = COMMANDES_MODE_MANUEL.find(commandKey);
    if (it != COMMANDES_MODE_MANUEL.end())
    {
        // Get the corresponding command
        std::string command = it->second;

        // Append the value if provided
        if (!value.empty())
        {
            command += " " + value;
        }

        // Write the command to the robot node
        opcua::NodeId robotNode(4, "|var|UHX65A.Application.User_PRG.Trame_IN_Akeros");

        opcua::String trame_manuelle(command);

        opcua::Variant trame_manuelle_var;
        trame_manuelle_var.setScalarCopy(trame_manuelle);

        // Write the trame to trame_in Node
        client.getNode(robotNode).writeValueScalar(trame_manuelle);

        logMessage << "Commande manuelle écrite correctement: " << command << std::endl;
        log(client, opcua::LogLevel::Trace, opcua::LogCategory::Server, logMessage.str());
    }
    else
    {
        std::cerr << "Error: Command key not found in the map." << std::endl;
    }
}

// Function to send continuous manual movement commands for a specified duration
void robot_manuel_move(opcua::Client &client, const std::string &axis, bool forward, const std::string &speed, int durationSeconds)
{
    opcua::NodeId robotNode(4, "|var|UHX65A.Application.User_PRG.Trame_IN_Akeros");
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::milliseconds(durationSeconds);
    opcua::String trame_manuelle;

    // Check if the axis exists in the map
    auto it = COMMANDES_ROBOT_MANUEL.find(axis);
    if (it != COMMANDES_ROBOT_MANUEL.end())
    {
        // Get the corresponding command
        std::string command = it->second;

        // Add direction and speed if provided
        if (forward)
        {
            command += " + " + speed;
        }
        else
        {
            command += " - " + speed;
        }

        // Write the command to the robot node

        // std::cout << command << std::endl;

        trame_manuelle = opcua::String(command);
        std::cout << trame_manuelle << std::endl;
        opcua::Variant trame_manuelle_var;
        trame_manuelle_var.setScalarCopy(trame_manuelle);

        // logMessage << "Commande robot manuelle écrite correctement: " << command << std::endl;
        // log(client, opcua::LogLevel::Trace, opcua::LogCategory::Server, logMessage.str());
    }
    else
    {
        std::cerr << "Error: Axis not found in the map." << std::endl;
    }

    // Continue sending commands until the specified duration is reached
    while (std::chrono::steady_clock::now() < endTime)
    {
        // Send the manual movement command
        client.getNode(robotNode).writeValueScalar(trame_manuelle);

        // Add a delay between consecutive commands (adjust as needed)
        // std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100 milliseconds delay
    }
}

void mouvement_manuel_test(opcua::Client &client)
{
    robot_manuel_move(client, "Y", false, "050", 3000); // Move X-axis forward for 3 seconds
}

void etat_robot_print(opcua::Client &client)
{
    etat_robot_get(client);

    std::stringstream logMessage;
    logMessage << "État du robot: " << EN_Robot_State[current_robot_state] << std::endl;
    log(client, opcua::LogLevel::Debug, opcua::LogCategory::Server, logMessage.str());
}

// Function to write manual command to the robot node
void rangement_robot(opcua::Client &client)
{
    std::stringstream logMessage;

    // Get the corresponding command
    std::string command = "MS A R 00";

    // Write the command to the robot node
    opcua::NodeId robotNode(4, "|var|UHX65A.Application.User_PRG.Trame_IN_Akeros");

    opcua::String trame_manuelle(command);

    opcua::Variant trame_manuelle_var;
    trame_manuelle_var.setScalarCopy(trame_manuelle);

    // Write the trame to trame_in Node
    client.getNode(robotNode).writeValueScalar(trame_manuelle);

    logMessage << "Rangement du robot.. " << command << std::endl;
    log(client, opcua::LogLevel::Trace, opcua::LogCategory::Server, logMessage.str());
}

void switch_operational_mode()
{
    int newMode;
    std::cout << "Enter the new operational mode (0 for GROUPS, 1 for SECTIONS): ";
    std::cin >> newMode;

    auto it = OPERATIONAL_MODE.find(newMode);
    if (it != OPERATIONAL_MODE.end())
    {
        currentOperationalMode = newMode;
        std::cout << "Operational mode switched to: " << OPERATIONAL_MODE[currentOperationalMode] << std::endl;
    }
    else
    {
        std::cout << "Invalid mode. No changes made." << std::endl;
    }
}
// A PRENDRE EN CHARGE (PAS URGENT) ------------------------
void vitesse_robot_get(opcua::Client &client)
{
    printf("VITESSE_ROBOT : Not handled yet.");
}

void repere_tole_get(opcua::Client &client)
{
    printf("REPERE_TOLE : Not handled yet.");
}
//--------------------------------------------------------

int runMenu(opcua::Client &client, std::vector<std::string> trames)
{
    bool running = true;
    int choice = 0;

    // Create a vector to store menu items
    std::vector<std::string> menuItems = {
        "Changement mode operations tôle                    ",
        "État des alarmes                                   ",
        "Texte de l'alarme                                  ",
        "Vitesse du robot (en %)                            ",
        "État du robot                                      ",
        "Position du robot                                  ",
        "Repere Tôle                                        ",
        "Lancement de mission (Tôle entiere)                ",
        "Lancement de mission (Trame Manuelle)              ",
        "Trame_Out GET                                     ",
        "Mouvement manuel Test                             ",
        "Rangement robot                                   ",
        "QUIT PROGRAM                                      " // Doit imperativement rester en derniere position
    };

    // Create a vector of function pointers
    std::vector<std::function<void()>> menuFunctions = {
        [&]
        { switch_operational_mode(); },
        [&]
        { etat_alarmes_get(client); },
        [&]
        { texte_alarme_get(client); },
        [&]
        { vitesse_robot_get(client); },
        [&]
        { etat_robot_print(client); },
        [&]
        { position_robot_get(client); },
        [&]
        { repere_tole_get(client); },
        [&]
        { mission_lancement(client, trames); },
        [&]
        { mission_lancement_une_trame(client); },
        [&]
        { trame_out_get(client); },
        [&]
        { mouvement_manuel_test(client); },
        [&]
        { rangement_robot(client); }};

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

//------------------------------------------- MAIN FUNCTION -----------------------

int main(int argc, char *argv[])
{

    // Vérification du nombre d'arguments de la ligne de commande
    if (argc != 2)
    {
        // Affiche un message d'erreur si l'argument n'est pas correct
        std::cerr << "Usage: " << argv[0] << " filename.drg" << std::endl;
        return 1; // Quitte le programme avec un code d'erreur
    }

    // Récupération du nom de fichier à partir des arguments de la ligne de commande
    std::string filename(argv[1]);
    std::stringstream logMessage;

    software_intro_section();

    //------------------------------------------- OPCUA CONFIG AND CONNECTION -----------------------

    opcua::Client client;
    client.setLogger([](auto level, auto category, auto msg)
                     {
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
              << resetColor << msg << std::endl; });

    std::cout << "Client Object Created!" << std::endl;

    // COMMENT THOSE LINES IF IN TEST MODE

    // client.connect("opc.tcp://192.168.100.14:4840");

    // // client.connect("opc.tcp://192.168.10.4:4840");

    // opcua::Node node = client.getNode(opcua::VariableId::Server_ServerStatus_CurrentTime);
    // const auto dt = node.readValueScalar<opcua::DateTime>();

    // std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;

    // END COMMENT THOSE LINES IF IN TEST MODE

    //------------------------------------------- END OPCUA CONFIG AND CONNECTION -----------------------

    // std::vector<std::string> liste_trames = driller_frames_execute(filename);

    // std::vector<SymValueGroup> final_values = driller_frames_execute(filename, currentOperationalMode);

    currentOperationalMode = 1;
    SymValueVariant result = driller_frames_execute(filename, currentOperationalMode);

    // Access the result based on its type
    if (std::holds_alternative<std::vector<SymValueGroup>>(result))
    {
        // Handle SymValueGroup vector
        std::vector<SymValueGroup> groups = std::get<std::vector<SymValueGroup>>(result);
        // Process SymValueGroup...
    }
    else if (std::holds_alternative<std::vector<SymValueSections>>(result))
    {
        // Handle SymValueSections vector
        std::vector<SymValueSections> final_values_sections = std::get<std::vector<SymValueSections>>(result);
        // Print the result
        for (const SymValueSections &section : final_values_sections)
        {
            std::cout << "Section " << section.section << ":" << std::endl;
            for (const SymValueGroup &group : section.groups)
            {
                std::cout << "  Groupe " << group.type << ":" << std::endl;
                for (const SymValue &value : group.values)
                {
                    std::cout << "    OPERATION: " << OPERATIONS_TYPES[value.type] << " {"
                              << "X: " << value.x
                              << ", Y: " << value.y
                              << ", Couleur: " << value.couleur
                              << ", Diametre: " << value.rayon * 2
                              << "}" << std::endl;
                }
            }
        }
    }

    // for (const SymValueGroup &group : final_values){
    // // Assuming the group type corresponds to the operation type (OP number)
    // int operation_type = group.type;
    // for (const SymValue &operation : group.values){
    // command = generate_single_command(operation, operation_type, epaisseur_tole, toolBank_json);

    // }
    // }

    // if (liste_trames.empty())
    // {
    //     logMessage << "Liste de trames vide!";
    //     log(client, opcua::LogLevel::Error, opcua::LogCategory::Userland, logMessage.str());
    // }
    // else
    // {
    //     logMessage << "Liste de trames correctement remplie!";
    //     log(client, opcua::LogLevel::Info, opcua::LogCategory::Userland, logMessage.str());
    // }

    /*
        // Impression de toutes les trames (Test)
        for (const auto& trame : liste_trames) {
            std::cout << trame << std::endl;
        }

        */

    // int menu_exit_code = runMenu(client, liste_trames); // Lancement du menu DRILLER

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