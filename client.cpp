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

#include "driller_frames.h"

extern std::vector<Outil> toolBank;

int main(int argc, char* argv[]) {

    //---------------------------------------------------------------------
/*
        // Déclaration de deux vecteurs pour stocker les coordonnées transformées
    std::vector<SymValue> Transformation_Repere_Tole, Transformation_Repere_Robot;
    std::string epaisseur_tole;
    Outil currentTool = {};
    std::string imageData;
    unsigned width = 640; // Set the width of your image
    unsigned height = 480; // Set the height of your image
    std::string image_filename = "output.png"; // Set the desired output filename


    // Création d'une structure de coordonnées pour le repère de la tôle
    Coordinates repere_tole;
    repere_tole.x = 4000.0;
    repere_tole.y = 2000.0;
    repere_tole.theta = 180.0;

    // Vérification du nombre d'arguments de la ligne de commande
    if (argc != 2) {
        // Affiche un message d'erreur si l'argument n'est pas correct
        std::cerr << "Usage: " << argv[0] << " filename.drg" << std::endl;
        return 1; // Quitte le programme avec un code d'erreur
    }


    std::vector<Outil> toolMagasine;

    // Check s'il y a au moins 20 outils dans la banque
    if (toolBank.size() >= 20) {
        // Recupere les 20 premiers outils dans la banque
        toolMagasine.assign(toolBank.begin(), toolBank.begin() + 20);
    } else {
        // Pas assez d'outils dans la banque d'outils
        std::cerr << "Error: Not enough tools in the toolBank." << std::endl;
        return 2; // Quitte le programme avec un code d'erreur
        
    }

    // Récupération du nom de fichier à partir des arguments de la ligne de commande
    std::string filename(argv[1]);

    // Ouverture du fichier en lecture
    std::ifstream file(filename);
    if (!file) {
        // Affiche un message d'erreur si l'ouverture du fichier échoue
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1; // Quitte le programme avec un code d'erreur
    }

    epaisseur_tole = getThickness(filename);
    if (epaisseur_tole != "ERR") {
        std::cout << "Epaisseur: " << epaisseur_tole << std::endl;
    } else {
        std::cerr << "Epaisseur not found or an error occurred." << std::endl;
    }

    // Lecture du contenu complet du fichier dans une chaîne de caractères
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());


// Extract the first <Image> section with image-type="png"
    std::string imageSection = extractImageSection(fileContent);

    if (!imageSection.empty()) {
        //std::cout << "Found Image Section:" << std::endl;
        //std::cout << imageSection << std::endl;

        // Extract the CDATA content from the Image section
        imageData = extractImageData(imageSection);
        if (!imageData.empty()) {
            std::cout << "Found CDATA Content:" << std::endl;
            //std::cout << imageData.length() << std::endl;

            std::string imageData_cleaned = cleanString(imageData);

            std::cout << imageData_cleaned.length() << std::endl;

            std::cout << "Last 10 characters of imageData: ";
            std::string::size_type length = imageData_cleaned.length();
            if (length >= 10) {
                std::cout.write(imageData.c_str() + length - 10, 10);
            } else {
                std::cout << imageData; // Print the entire string if it's less than 10 characters
            }
            std::cout << std::endl;
            // Decode base64-encoded image data
            //std::string decodedImageData = base64_decode(imageData);

            //std::string decodedImageData = b64decode(imageData, imageData.size());

            std::vector<BYTE> decodedImageData = base64_decode(imageData_cleaned);

            // Print the size of the decodedImageData vector
            std::cout << "Size of decodedImageData: " << decodedImageData.size() << " bytes" << std::endl;

                // Save the image data as a PNG file
                saveImageToPNG(decodedImageData, width, height, image_filename);



                        
        } else {
            std::cout << "No CDATA content found in the Image section." << std::endl;
        }
    } else {
        std::cout << "No image section with image-type=\"png\" found in the file." << std::endl;
    }







    // Extraction des chemins et des coordonnées à partir du contenu du fichier
    std::vector<PathCoordinates> pathCoordinatesList = extractPathAndCoordinates(fileContent);

    // Vérification si la liste des coordonnées est vide
    if (pathCoordinatesList.empty()) {
        std::cout << "Aucun contenu dans la section 200." << std::endl;
        return 1; // Quitte le programme avec un code d'erreur
    }

    // Affichage des chemins et des coordonnées
    printPathCoordinates(pathCoordinatesList);

    // Boucle pour gérer les fichiers .sym
    for (const PathCoordinates& pathCoordinates : pathCoordinatesList) {
        // Vérifie si le chemin se termine par ".sym"
        if (pathCoordinates.path.substr(pathCoordinates.path.size() - 4) == ".sym") {
            // Extraction des données brutes du fichier .sym
            std::vector<SymValue> raw_holes_data = extractSymData(pathCoordinates.path);
            
            // Vérifie si les données brutes ne sont pas vides
            if (!raw_holes_data.empty()) {



                // Transformation 1 : Transformation des coordonnées dans le repère de la tôle
                for (const Coordinates& coordinate : pathCoordinates.coordinates) {
                    for (const SymValue& value : raw_holes_data) {
                        // Applique la transformation dans le repère de la tôle
                        SymValue transformedValue = transformation_repere_rad(value, coordinate);
                        // Ajoute les coordonnées transformées au vecteur Transformation_Repere_Tole
                        Transformation_Repere_Tole.push_back(transformedValue);
                    }
                }

                // Transformation 2 : Transformation des coordonnées dans le repère du robot
                for (const SymValue& value : Transformation_Repere_Tole) {
                    // Applique la transformation dans le repère du robot
                    SymValue transformedValue = transformation_repere_rad(value, repere_tole);
                    // Ajoute les coordonnées transformées au vecteur Transformation_Repere_Robot
                    Transformation_Repere_Robot.push_back(transformedValue);
                }

                // Tri des données transformées dans le repère robot
                std::vector<int> customTypeOrder = {1, 4, 2, 3, 5};
                std::vector<SymValueGroup> final_values = sortAndGroupByType(Transformation_Repere_Robot, customTypeOrder);
                for (SymValueGroup& group : final_values) {
                std::sort(group.values.begin(), group.values.end(), sortByRayonAndDistance);
                }


                // Affichage des données transformées (test)
                //for (SymValueGroup& group : final_values) {
                //printStructure(group.values);
                //}

                // Call the generateCommands function
                generateCommands(final_values, epaisseur_tole);

                
            }
        }
    }


*/

//VARIABLES AKEROS
std::string numero_alarme = "1"; // Replace with the actual alarm number as a string
std::string etat_alarme_str = "UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + numero_alarme + "].Active";
std::string texte_alarme_str = "UHX65A.Application.Alarm_Global.Alarm.Active_Alarm[" + numero_alarme + "].Text";

UA_NodeId etat_alarme = UA_NODEID_STRING(4, const_cast<char*>(etat_alarme_str.c_str()));
UA_NodeId texte_alarme = UA_NODEID_STRING(4, const_cast<char*>(texte_alarme_str.c_str()));
UA_NodeId vitesse_robot = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.GVL_Config.Speed"));
UA_NodeId etat_robot = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Robot_Cantilever.OUT.State"));
UA_NodeId position_robot = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Robot_Cantilever.OUT.Position"));
UA_NodeId repere_tole_opcua = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Robot_Cantilever.Repere_tole"));
UA_NodeId reception_mission = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Trame_IN_Akeros"));
UA_NodeId echo_fin_mission = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.Trame_OUT_Akeros"));
UA_NodeId lancement_mission = UA_NODEID_STRING(4, const_cast<char*>("UHX65A.Application.User_PRG.RAZ_Trame_IN_Akeros"));

//--------------------




    
    //------------------------------------------------------------------------
    //OPCUA SECTION
    /*
    opcua::Client client; //Création d'un nouveau objet de type opcua::Client
    std::cout << "Client Object Created!" << std::endl;
    client.connect("opc.tcp://192.168.100.14:4840"); // Tentative de connexion au client via son addresse tcp

    opcua::Node node = client.getNode(opcua::VariableId::Server_ServerStatus_CurrentTime); //
    const auto dt = node.readValueScalar<opcua::DateTime>();

    std::cout << "Server date from the PLC (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
*/
    // Create a client instance
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *clientConfig = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(clientConfig);

    // Connect to the server
    const char *serverUrl = "opc.tcp://192.168.100.14:4840";
    UA_StatusCode connectStatus = UA_Client_connect(client, serverUrl);

    if (connectStatus != UA_STATUSCODE_GOOD) {
        // Handle connection error
        UA_Client_delete(client); // Clean up the client if the connection fails
        return -1; // Or some other error handling
    }

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

   //STATUS INFORMATION GATHERING SECTION (UA_Client_readValueAttribute TO FIX)
//STATUS INFORMATION GATHERING SECTION (UA_Client_readValueAttribute TO FIX)
UA_DataValue dataValue;
UA_StatusCode readStatus = UA_Client_readValueAttribute(client, etat_robot, &dataValue.value);

if (readStatus == UA_STATUSCODE_GOOD) {
    // Successfully read the value. You can access the value using dataValue.value.
    if (dataValue.hasValue) {
        UA_Variant variantValue = dataValue.value;
        if (variantValue.type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String valueString = *(UA_String*)variantValue.data;
            printf("Value of etat_robot: %.*s\n", (int)valueString.length, valueString.data);
        } else {
            // Handle data type mismatch or other cases as needed.
        }
    }
} else {
    // Handle read error.
    printf("Error reading etat_robot: %s\n", UA_StatusCode_name(readStatus));
}

    //---------------------



    

    
}