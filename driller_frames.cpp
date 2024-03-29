// BUILD : g++ -std=c++11 -o driller_frames driller_frames.cpp
// EXECUTE : ./driller_frames [filename.drg]

#include <iostream> // Inclut la bibliothèque d'entrée/sortie
#include <fstream>  // Inclut la bibliothèque pour la lecture/écriture de fichiers
#include <string>   // Inclut la bibliothèque pour manipuler les chaînes de caractères
#include <vector>   // Inclut la bibliothèque pour utiliser des vecteurs
#include <variant>
#include <sstream>   // Inclut la bibliothèque pour manipuler des flux de chaînes de caractères
#include <algorithm> // Inclut la bibliothèque pour utiliser la fonction std::sort
#include <cstdlib>
#include <chrono>
#include <ctime>
// Bibliotheques Windows et Linux
#include <iomanip>
#include <limits>
#include <unordered_map>
#include <climits>
#include <cmath>

// Librairie de gestion des formats JSON pour la banque d'outils
#include <json.hpp>
#include <Python.h>
// for convenience
using json = nlohmann::json;

#include "driller_frames.h" // Inclut le header avec les prototypes des fonctions de generation de trames

std::string epaisseur_tole;

// Ordre du tri par groupe d'operations
std::vector<int> customTypeOrder = {1, 4, 2, 3, 5};

// BANQUE D'OUTILS
// std::vector<Outil> toolBank = {
//     // Perçage (Classe D)
//     {800.0, 5.0, 100.0, "D", 5.0, 60.0, 45.0, 0},
//     {800.0, 5.0, 100.0, "D", 6.0, 60.0, 45.0, 0},
//     {800.0, 5.0, 100.0, "D", 7.0, 60.0, 45.0, 0},
//     {800.0, 5.0, 100.0, "D", 8.0, 60.0, 45.0, 0},
//     {800.0, 5.0, 100.0, "D", 8.5, 60.0, 45.0, 0},
//     {1500.0, 8.0, 200.0, "D", 9.0, 75.0, 90.0, 0},
//     {1500.0, 8.0, 200.0, "D", 10.0, 75.0, 90.0, 0},
//     {1200.0, 6.0, 200.0, "D", 12.0, 75.0, 60.0, 0},
//     {1200.0, 6.0, 200.0, "D", 14.0, 75.0, 60.0, 0},
//     // Fraisurage (Classe C)
//     {1000.0, 5.0, 100.0, "C", 10.0, 50.0, 90.0, 0, 0.0},
//     {1500.0, 7.0, 200.0, "C", 20.0, 75.0, 60.0, 0, 10.0},
//     // Taraudage (Classe T)
//     {800.0, 5.0, 125.0, "T", 6.0, 60.0, 45.0, 0},
//     {1000.0, 6.0, 200.0, "T", 8.0, 75.0, 60.0, 0},
//     {1500.0, 8.0, 200.0, "T", 10.0, 75.0, 90.0, 0},
//     // Lamage (Classe S)
//     {800.0, 5.0, 100.0, "S", 6.0, 60.0, 45.0, 0},
//     {1000.0, 6.0, 200.0, "S", 8.0, 75.0, 60.0, 0},
//     {1200.0, 8.0, 200.0, "S", 10.0, 75.0, 90.0, 0},
//     {800.0, 5.0, 100.0, "S", 7.0, 60.0, 45.0, 0},
//     {1000.0, 6.0, 200.0, "S", 9.0, 75.0, 60.0, 0},
//     {1200.0, 8.0, 200.0, "S", 11.0, 75.0, 90.0, 0},
// };

std::string get_epaisseur_tole()
{
    return epaisseur_tole;
}

// Function to read the tool bank from a JSON file
std::vector<Outil> readToolBank(const std::string &filename)
{
    std::vector<Outil> toolBank; // Vector to store the tools

    try
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        json j;
        file >> j;

        // Check if the top-level structure is an array
        if (!j.is_array())
        {
            throw std::invalid_argument("JSON file should contain an array of tools.");
        }

        // Iterate over array elements
        for (const auto &toolJson : j)
        {
            Outil tool;
            tool.vitesse_rotation = toolJson.at("vitesse_rotation");
            tool.vitesse_avance = toolJson.at("vitesse_avance");
            tool.pas_outil = toolJson.at("pas_outil");
            tool.classe = toolJson.at("classe");
            tool.diametre = toolJson.at("diametre");
            tool.longueur_pilote = toolJson.at("longueur_pilote");
            tool.angle = toolJson.at("angle");
            tool.nombre_cycles = toolJson.at("nombre_cycles");
            tool.diametre_min = toolJson.at("diametre_min");
            tool.diametre_max = toolJson.at("diametre_max");

            toolBank.push_back(tool);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reading tool bank: " << e.what() << std::endl;
    }

    return toolBank;
}

// Function to clean a string by removing white spaces and empty characters
std::string cleanString(const std::string &input)
{
    std::string result = input;

    // Remove white spaces (spaces, tabs, newlines, etc.)
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());

    // Remove empty characters (null characters, etc.)
    result.erase(std::remove_if(result.begin(), result.end(), ::iscntrl), result.end());

    return result;
}

// Function to extract all <Image> sections with image-type="png"
std::vector<std::string> extractAllImageSections(const std::string &content)
{
    std::vector<std::string> imageSections;
    size_t pos = 0;

    while ((pos = content.find("<Image image-type=\"png\"", pos)) != std::string::npos)
    {
        // Find the end of the current <Image> section
        size_t endPos = content.find("</Image>", pos);
        if (endPos != std::string::npos)
        {
            // Extract the <Image> section including attributes and CDATA content
            imageSections.push_back(content.substr(pos, endPos - pos + 9)); // +9 to include </Image>
            pos = endPos + 9;                                               // Move pos to the end of the current <Image> section
        }
        else
        {
            break; // No more <Image> sections found
        }
    }

    return imageSections;
}

// Function to extract the CDATA content from an <Image> section
std::string extractImageData(const std::string &imageSection)
{
    std::string imageData;

    // Find the beginning of the CDATA section
    size_t cdataStart = imageSection.find("<![CDATA[");
    if (cdataStart != std::string::npos)
    {
        // Find the end of the CDATA section
        size_t cdataEnd = imageSection.find("]]>", cdataStart);
        if (cdataEnd != std::string::npos)
        {
            // Extract the CDATA content
            imageData = imageSection.substr(cdataStart + 9, cdataEnd - (cdataStart + 9)); // +9 to skip "<![CDATA["
        }
    }

    return imageData;
}

Outil selectTool(double rayon, const std::string &classe, std::vector<Outil> tool_bank)
{
    Outil selectedTool;
    bool armSlotEmpty = tool_bank[0].classe == "Empty";

    for (int i = 0; i < tool_bank.size(); ++i)
    {

        // Check if the tool matches the criteria
        if (tool_bank[i].classe == classe)
        {
            double toolDiameter = tool_bank[i].diametre;
            double toolDiameterMin = tool_bank[i].diametre_min;
            double toolDiameterMax = tool_bank[i].diametre_max;

            if (classe == "C")
            {
                // For class "C", choose a tool if rayon*2 is within the tool's diameter range
                if (rayon * 2 >= toolDiameterMin && rayon * 2 <= toolDiameterMax)
                {
                    selectedTool = tool_bank[i]; // Convert JSON to Outil
                    break;                       // Stop after selecting the first suitable tool
                }
            }
            else
            {
                // For other classes, choose a tool if rayon*2 is exactly equal to the tool's diameter
                if (rayon * 2 == toolDiameter)
                {
                    selectedTool = tool_bank[i]; // Convert JSON to Outil
                    break;                       // Stop after selecting the first suitable tool
                }
            }
        }
    }

    return selectedTool;
}

void updateJsonFile(const std::vector<Outil> &tool_bank)
{
    json j;

    for (const auto &tool : tool_bank)
    {
        json toolJson = {
            {"vitesse_rotation", tool.vitesse_rotation},
            {"vitesse_avance", tool.vitesse_avance},
            {"pas_outil", tool.pas_outil},
            {"classe", tool.classe},
            {"diametre", tool.diametre},
            {"longueur_pilote", tool.longueur_pilote},
            {"angle", tool.angle},
            {"nombre_cycles", tool.nombre_cycles},
            {"diametre_min", tool.diametre_min},
            {"diametre_max", tool.diametre_max}};

        j.push_back(toolJson);
    }

    try
    {
        // Open the JSON file for writing
        std::ofstream jsonFile("./tool_bank.json");
        if (!jsonFile.is_open())
        {
            throw std::runtime_error("Unable to open JSON file for writing.");
        }

        // Write the JSON data to the file
        jsonFile << std::setw(4) << j << std::endl;

        // Close the file
        jsonFile.close();

        // std::cout << "Tool bank updated successfully." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error updating JSON file: " << e.what() << std::endl;
    }
}

// Function to find the position of a tool in the toolBank vector
int findToolPosition(const Outil &tool, std::vector<Outil> tool_bank)
{
    for (int i = 0; i < tool_bank.size(); ++i)
    {
        if (tool_bank[i].vitesse_rotation == tool.vitesse_rotation &&
            tool_bank[i].vitesse_avance == tool.vitesse_avance &&
            tool_bank[i].pas_outil == tool.pas_outil &&
            tool_bank[i].classe == tool.classe &&
            tool_bank[i].diametre == tool.diametre &&
            tool_bank[i].longueur_pilote == tool.longueur_pilote &&
            tool_bank[i].angle == tool.angle)
        {
            return i; // Found the tool, return its position
        }
    }
    return -99; // Tool not found in the vector
}

// Fonction utilitaire pour extraire le nom du fichier du chemin complet
std::string getFilenameFromPath(const std::string &path)
{
    // Trouve la dernière occurrence de '/' ou '\' dans le chemin
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos)
    {
        // Retourne la partie du chemin après la dernière occurrence
        return path.substr(lastSlash + 1);
    }
    // Si aucun '/' ou '\' n'est trouvé, retourne le chemin complet comme nom de fichier
    return path;
}

std::string formatNumber(double value, int digits)
{
    // Format the value as a 3-digit string with leading zeros
    std::stringstream ss;
    ss << std::setw(digits) << std::setfill('0') << value;
    return ss.str();
}

// Function to retrieve the 'thickness' attribute from a .drg file as a 3-digit string
std::string getThickness(const std::string &filePath)
{
    std::ifstream file(filePath);
    std::string line;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            size_t pos = line.find("name=\"Epaisseur\"");
            if (pos != std::string::npos)
            {
                // Found the line containing "name=\"Epaisseur\""
                pos = line.find("value=\"");
                if (pos != std::string::npos)
                {
                    // Found the line containing "value=\""
                    pos += 7; // Move to the beginning of the actual value
                    size_t endPos = line.find("\"", pos);
                    if (endPos != std::string::npos)
                    {
                        // Found the closing quote for the value
                        std::string valueStr = line.substr(pos, endPos - pos);
                        try
                        {
                            // Convert the value to an integer (assuming it's in cm)
                            int valueCm = std::stoi(valueStr);
                            // Convert cm to mm (1 cm = 10 mm)
                            int valueMm = valueCm * 10;
                            // Format the value as a 3-digit string with leading zeros
                            std::stringstream ss;
                            ss << std::setw(3) << std::setfill('0') << valueMm;
                            return ss.str();
                        }
                        catch (const std::exception &e)
                        {
                            // Handle conversion error (e.g., value is not an integer)
                            std::cerr << "Error parsing 'value' attribute: " << e.what() << std::endl;
                            return "ERR"; // Error code as a string
                        }
                    }
                }
            }
        }
        file.close();
    }
    else
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
    }

    return "N/A"; // Value not found or an error occurred, return "N/A"
}

// Fonction pour extraire les chemins et les coordonnées à partir d'une chaîne de contenu
std::vector<PathCoordinates> extractPathAndCoordinates(const std::string &content)
{
    // Initialise une liste de PathCoordinates vide
    std::vector<PathCoordinates> pathCoordinatesList;

    // Recherche le début de l'attribut num=200 dans le contenu
    size_t attr200Start = content.find("<Attr num=\"200\" name=\"AKEROS\"");
    if (attr200Start == std::string::npos)
    {
        // Affiche un message d'erreur si l'attribut num=200 n'est pas trouvé
        std::cerr << "Attribute num=200 not found in the file." << std::endl;
        return pathCoordinatesList;
    }

    // Recherche le début de la section "value" dans l'attribut num=200
    size_t valueStart = content.find("value=\"", attr200Start);
    if (valueStart == std::string::npos)
    {
        // Affiche un message d'erreur si la section "value" n'est pas trouvée
        std::cerr << "Value section not found for Attribute num=200." << std::endl;
        return pathCoordinatesList;
    }

    // Déplace le curseur vers le début des données de la section "value"
    valueStart += 7; // 7 caractères pour atteindre le début des données

    // Recherche la fin de la section "value"
    size_t valueEnd = content.find("\"", valueStart);
    if (valueEnd == std::string::npos)
    {
        // Affiche un message d'erreur si la section "value" n'est pas terminée correctement
        std::cerr << "Value section not terminated correctly for Attribute num=200." << std::endl;
        return pathCoordinatesList;
    }

    // Extrait le contenu de la section "value" entre les guillemets
    std::string valueSection = content.substr(valueStart, valueEnd - valueStart);

    // Délimiteur utilisé pour séparer les éléments dans la section "value"
    std::string delimiter = ";";
    size_t pos = 0;

    // Boucle pour traiter chaque élément séparé par le délimiteur
    while ((pos = valueSection.find(delimiter)) != std::string::npos)
    {
        // Extrait un élément de la section "value"
        std::string section = valueSection.substr(0, pos);
        if (section.substr(0, 2) == "\\\\")
        {
            // Si l'élément commence par "\\"
            PathCoordinates pathCoordinates;
            pathCoordinates.path = section;
            pathCoordinatesList.push_back(pathCoordinates);
        }
        else
        {
            // Si l'élément n'est pas une chaîne commençant par "\\"
            // Convertit la chaîne de coordonnées en une structure Coordinates
            std::istringstream iss(section);
            double x, y, theta;
            char coordDelimiter = '|';

            // Ignore le caractère '[' initial
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');

            // Extrait les valeurs x, y et theta
            if (iss >> x >> coordDelimiter >> y >> coordDelimiter >> theta)
            {
                // Crée une structure Coordinates et ajoute-la à la liste
                Coordinates coord;
                coord.x = x;
                coord.y = y;
                coord.theta = theta;
                pathCoordinatesList.back().coordinates.push_back(coord);
            }

            // Ignore les caractères ']' et ';' pour passer à l'élément suivant
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ';');
        }
        // Supprime l'élément traité de la section "value"
        valueSection.erase(0, pos + delimiter.length());
    }

    // Traite la coordonnée restante s'il y en a une
    if (!valueSection.empty() && valueSection.substr(0, 2) != "\\\\")
    {
        // Convertit la chaîne de coordonnées en une structure Coordinates
        std::istringstream iss(valueSection);
        double x, y, theta;
        char coordDelimiter = '|';

        // Ignore le caractère '[' initial
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');

        // Extrait les valeurs x, y et theta
        if (iss >> x >> coordDelimiter >> y >> coordDelimiter >> theta)
        {
            // Crée une structure Coordinates et ajoute-la à la liste
            Coordinates coord;
            coord.x = x;
            coord.y = y;
            coord.theta = theta;
            pathCoordinatesList.back().coordinates.push_back(coord);
        }

        // Ignore les caractères ']' et ';' pour passer à l'élément suivant
        iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
        iss.ignore(std::numeric_limits<std::streamsize>::max(), ';');
    }

    // Retourne la liste des chemins et des coordonnées
    return pathCoordinatesList;
}

// Fonction pour extraire la section "200" d'un fichier .sym
std::string extractSym200(const std::string &symFileContent)
{
    // Recherche la balise d'ouverture de la section "200"
    std::string openingTag = "<Attr num=\"200\"";
    size_t openingPos = symFileContent.find(openingTag);
    if (openingPos == std::string::npos)
    {
        // Si la section "200" n'est pas trouvée, retourne une chaîne vide
        return "";
    }

    // Recherche la balise de fermeture de la section "200"
    size_t closingPos = symFileContent.find("</Attr>", openingPos);
    if (closingPos == std::string::npos)
    {
        // Si la section "200" n'est pas terminée correctement, retourne une chaîne vide
        return "";
    }

    // Extrait le contenu de la section "200" avec les balises
    std::string section200 = symFileContent.substr(openingPos, closingPos - openingPos + 8);

    return section200;
}

// Fonction pour afficher les chemins et les coordonnées
void printPathCoordinates(std::vector<PathCoordinates> &pathCoordinatesList)
{
    int pathCount = 1;
    for (auto &pathCoordinates : pathCoordinatesList)
    {
        // Supprime les caractères de nouvelle ligne de pathCoordinates.path
        size_t pos;
        while ((pos = pathCoordinates.path.find('\n')) != std::string::npos)
        {
            pathCoordinates.path.erase(pos, 1);
        }

        // Affiche le numéro de pièce et le chemin
        std::cout << "Pièce " << pathCount << " : " << pathCoordinates.path << std::endl;
        std::cout << "Coordonnées des exemplaires (pièce " << pathCount << ") : " << std::endl;

        // Itère sur les coordonnées de pathCoordinates.coordinates (un vecteur de Coordinates)
        for (const Coordinates &coord : pathCoordinates.coordinates)
        {
            std::cout << "x: " << coord.x << ", y: " << coord.y << ", theta: " << coord.theta << std::endl;
        }

        pathCount++;
    }
}

// Fonction pour extraire les données SymValue à partir de la section "value"
std::vector<SymValue> parseValueSection(const std::string &valueSection)
{
    std::vector<SymValue> symValues;

    // Recherche la position de l'attribut "value"
    size_t pos = valueSection.find("value=\"");
    if (pos == std::string::npos)
    {
        // Affiche un message d'erreur si l'attribut "value" n'est pas trouvé
        // std::cerr << "Aucune opération à effectuer." << std::endl;
        return symValues; // Retourne un vecteur vide en cas d'erreur
    }

    // Avance la position au début des valeurs
    pos += 7; // Avance de 7 caractères pour atteindre le début des valeurs

    // Recherche la position de la fermeture des guillemets
    size_t endPos = valueSection.find("\"", pos);
    if (endPos == std::string::npos)
    {
        // Affiche un message d'erreur si la fermeture des guillemets n'est pas trouvée
        std::cerr << "Closing quote for the value attribute not found." << std::endl;
        return symValues; // Retourne un vecteur vide en cas d'erreur
    }

    // Extrait le contenu de l'attribut "value" entre les guillemets
    std::string valueStr = valueSection.substr(pos, endPos - pos);

    // Initialise un flux de chaînes de caractères à partir de la chaîne de valeurs
    std::istringstream valueStream(valueStr);
    std::string token;

    // Boucle pour extraire et analyser chaque élément séparé par '|'
    while (std::getline(valueStream, token, '|'))
    {
        // Initialise un flux de chaînes de caractères à partir de l'élément
        std::istringstream tokenStream(token);
        SymValue symValue;
        char separator;

        // Lit les valeurs de l'élément et les stocke dans la structure SymValue
        tokenStream >> symValue.x >> separator >> symValue.y >> separator >> symValue.z >> separator >> symValue.type >> separator >> symValue.couleur >> separator >> symValue.rayon;

        if (tokenStream.fail())
        {
            // Affiche un message d'erreur en cas d'échec de l'analyse de l'élément
            std::cerr << "Error parsing token: " << token << std::endl;
            continue; // Passe à l'élément suivant en cas d'erreur
        }

        // Ajoute la structure SymValue au vecteur
        symValues.push_back(symValue);
    }

    return symValues;
}

// Fonction pour extraire les données SymValue à partir d'un fichier .sym
std::vector<SymValue> extractSymData(const std::string &symFilePath)
{
    // Initialise un vecteur de SymValue pour stocker les données extraites
    std::vector<SymValue> parsedValues;

    // Extrait le nom de fichier du chemin complet
    std::string filename = getFilenameFromPath(symFilePath);

    // Ouvre le fichier .sym
    std::ifstream symFile(filename);
    if (!symFile)
    {
        // Affiche un message d'erreur en cas d'échec de l'ouverture du fichier
        std::cerr << "Error opening .sym file: " << symFilePath << std::endl;
    }

    // Lit le contenu complet du fichier .sym dans une chaîne de caractères
    std::string symFileContent((std::istreambuf_iterator<char>(symFile)), std::istreambuf_iterator<char>());

    // Extrait le contenu de la section "200"
    std::string section200Content = extractSym200(symFileContent);

    // Vérifie si la section "200" est présente
    if (!section200Content.empty())
    {
        // Affiche un message indiquant les opérations à effectuer pour chaque exemplaire de la pièce
        // std::cout << "Operations à effectuer pour chaque exemplaire de la pièce -> " << filename << std::endl;

        // Appelle la fonction pour analyser la section "value" et stocker les données dans parsedValues
        parsedValues = parseValueSection(section200Content);

        // Boucle for utilisée pour afficher les valeurs (commentée car elle est en cours de développement)
        // for (const SymValue& value : parsedValues) {
        //    std::cout << "[" + std::to_string(value.x)+ "," + std::to_string(value.y) + "," + std::to_string(value.z) + "," + std::to_string(value.type) + "," + std::to_string(value.couleur) + "," + std::to_string(value.rayon) + "]" << std::endl;
        // }
    }
    else
    {
        // Affiche un message d'erreur si la section "200" n'est pas trouvée
        std::cerr << "Section 200 not found in .sym file: " << symFilePath << std::endl;
    }

    // Retourne le vecteur de SymValues
    return parsedValues;
}

std::vector<SymValueGroup> sortAndGroupByType(const std::vector<SymValue> &inputValues, const std::vector<int> &typeOrder)
{
    // Initialize a vector of SymValueGroup
    std::vector<SymValueGroup> groupedValues;

    // Create a map to store the custom order for each type
    std::unordered_map<int, int> typeOrderMap;
    for (size_t i = 0; i < typeOrder.size(); ++i)
    {
        typeOrderMap[typeOrder[i]] = static_cast<int>(i);
    }

    // Sort the inputValues by type using the custom order
    std::vector<SymValue> sortedValues = inputValues;
    std::sort(sortedValues.begin(), sortedValues.end(), [&](const SymValue &a, const SymValue &b)
              {
        int typeAOrder = typeOrderMap.find(a.type) != typeOrderMap.end() ? typeOrderMap[a.type] : INT_MAX;
        int typeBOrder = typeOrderMap.find(b.type) != typeOrderMap.end() ? typeOrderMap[b.type] : INT_MAX;

        if (typeAOrder != typeBOrder) {
            return typeAOrder < typeBOrder;
        }

        // If types have the same order, sort by type
        return a.type < b.type; });

    // Group the sorted values by type
    for (const SymValue &value : sortedValues)
    {
        int valueType = value.type;

        // Try to find a group with the same type
        auto groupIt = std::find_if(groupedValues.begin(), groupedValues.end(), [&](const SymValueGroup &group)
                                    { return group.type == valueType; });

        if (groupIt != groupedValues.end())
        {
            // Add the value to an existing group with the same type
            groupIt->values.push_back(value);
        }
        else
        {
            // Create a new group and add the value to it
            SymValueGroup newGroup;
            newGroup.type = valueType;
            newGroup.values.push_back(value);
            groupedValues.push_back(newGroup);
        }
    }

    return groupedValues;
}

bool compareSymValuesBySection(const SymValue &a, const SymValue &b)
{
    // Assuming 500 x 500 sections on the sheet metal
    int sectionA = static_cast<int>(a.x / 500) + static_cast<int>(a.y / 500) * 8; // Assuming 8 sections in x direction
    int sectionB = static_cast<int>(b.x / 500) + static_cast<int>(b.y / 500) * 8;

    return sectionA < sectionB;
}

std::vector<SymValueSections> sortAndGroupBySection(const std::vector<SymValue> &inputValues, const std::vector<int> &typeOrder)
{
    // Sort inputValues by sections
    std::vector<SymValue> sortedBySection = inputValues;
    std::sort(sortedBySection.begin(), sortedBySection.end(), compareSymValuesBySection);

    // Group values by sections
    std::vector<SymValueSections> groupedBySection;
    int currentSection = -1;

    for (const SymValue &value : sortedBySection)
    {
        int section = static_cast<int>((value.x / 500) + static_cast<int>(value.y / 500) * 8); // Assuming 8 sections in x direction

        if (section != currentSection)
        {
            // Start a new section
            SymValueSections newSection;
            newSection.section = section;
            newSection.groups = sortAndGroupByType({value}, typeOrder);
            groupedBySection.push_back(newSection);
            currentSection = section;
        }
        else
        {
            // Add to the current section
            groupedBySection.back().groups[0].values.push_back(value);
        }
    }

    return groupedBySection;
}

// Fonction de tri des données transformées
bool sortByRayonAndDistance(const SymValue &a, const SymValue &b)
{
    // First, sort by type
    if (a.type != b.type)
    {
        return a.type < b.type;
    }

    // Then, sort by 'rayon'
    if (a.rayon != b.rayon)
    {
        return a.rayon < b.rayon;
    }

    // Finally, sort by distance between (x, z) values
    double distanceA = std::sqrt(a.x * a.x + a.z * a.z);
    double distanceB = std::sqrt(b.x * b.x + b.z * b.z);

    return distanceA < distanceB;
}

// Fonction pour afficher les données SymValue
void printStructure(const std::vector<SymValue> &values)
{
    for (const SymValue &value : values)
    {
        // std::cout << "[" << value.x << "; " << value.y << "; " << value.z << "; << value.type << "; " << value.couleur << "; " << value.rayon << "]" << std::endl;
    }
}

int countDecimalPlaces(double value)
{
    std::ostringstream oss;
    oss << value;
    std::string str = oss.str();

    size_t decimalPos = str.find('.');
    if (decimalPos != std::string::npos)
    {
        return str.length() - decimalPos - 1;
    }
    else
    {
        return 0; // No decimal point found
    }
}

// FONCTIONS DE TRANSFORMATION (1 ET 2)
// ANGLE -> Theta des pièces
// POINT -> Coordonnées de l'opération de chaque exemplaire de la pièce
// ORIGINE -> Coordonnées de la pièce dans la tôle

// Resultat.X := (COS (Angle/57.2958) * Point.X) - (SIN (Angle/57.2958) * Point.Y) + Origine.x;
// Resultat.Y := (SIN (Angle/57.2958) * Point.X) + (COS (Angle/57.2958) * Point.Y) + Origine.Y;

// Fonction de transformation des coordonnées dans le repère de la tôle (en radians)
SymValue transformation_repere_rad(const SymValue &point, const Coordinates &repere)
{
    SymValue transformed = point;
    transformed.x = (cos(repere.theta / 57.2958) * point.x) - (sin(repere.theta / 57.2958) * point.y) + repere.x;
    transformed.y = (sin(repere.theta / 57.2958) * point.x) + (cos(repere.theta / 57.2958) * point.y) + repere.y;
    return transformed;
}

// ---------------- GENERATION DE LA TRAME ROBOT ---------------------------------- //

// Trame Percage (OP. 1)
std::string generatePercageCommand(const PercageParams &params)
{
    return "MS A D " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.commonParams.vitesseAvanceOutil;
}

// Trame Fraisurage (OP. 4)
std::string generateFraisurageCommand(const FraisurageParams &params)
{
    return "MS A C " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.commonParams.vitesseAvanceOutil +
           " " + params.angleFraise + " " + params.diametreExterieur;
}

// Trame Taraudage (OP. 2 et 3)
std::string generateTaraudageCommand(const TaraudageParams &params)
{
    return "MS A T " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.pasOutil;
}

// Trame Lamage (OP. 5)
std::string generateLamageCommand(const LamageParams &params)
{
    return "MS A S " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.commonParams.vitesseAvanceOutil +
           " " + params.profondeur + " " + params.longueurPiloteOutil;
}

// Function to generate commands for all elements in final_values
std::string generate_single_command(const SymValue &operation, int opType, const std::string &epaisseur_tole, std::vector<Outil> tool_bank)
{
    PercageParams nouveau_percage;
    FraisurageParams nouveau_fraisurage;
    TaraudageParams nouveau_taraudage;
    LamageParams nouveau_lamage;
    Outil outil;
    std::string tool_pos;
    int tool_pos_int;

    // Common parameters
    CommonParams commonParams;
    double opXtemp = std::floor(operation.x * 100) / 100;
    double opYtemp = std::floor(operation.y * 100) / 100;
    int decimalPlaces_X = countDecimalPlaces(opXtemp);
    int decimalPlaces_Y = countDecimalPlaces(opYtemp);
    commonParams.coordX = formatNumber((opXtemp * std::pow(10, decimalPlaces_X)), 6); // Convert to mm
    commonParams.coordY = formatNumber((opYtemp * std::pow(10, decimalPlaces_Y)), 6); // Convert to mm
    commonParams.epaisseurTole = epaisseur_tole;

    // Generate commands based on operation type
    switch (opType)
    {
    case 1: // Percage (OP. 1)
        outil = selectTool(operation.rayon, "D", tool_bank);
        std::cout << "Classe: " << outil.classe << std::endl;
        tool_pos_int = findToolPosition(outil, tool_bank);
        std::cout << "Position: " << tool_pos_int << std::endl;

        if (tool_pos_int == 0)
        {
            tool_pos = "00";
        }
        else
        {
            tool_pos = formatNumber(((tool_pos_int) + 1), 2);
        }
        if (tool_pos != "-98")
        {
            commonParams.emplacementOutil = tool_pos;
            commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation, 4);
            commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance * 100), 3);
            nouveau_percage.commonParams = commonParams;
            std::string percageCommand = generatePercageCommand(nouveau_percage);
            // Swap the positions in both the JSON file and the array
            std::swap(tool_bank[0], tool_bank[tool_pos_int]);
            // Update the JSON file with the swapped positions
            updateJsonFile(tool_bank);
            return percageCommand;
        }
        else
        {
            // std::cout << "No Tool Found!" << std::endl;
        }
        break;
    case 4: // Fraisurage (OP. 4)
        outil = selectTool(operation.rayon, "C", tool_bank);
        tool_pos_int = findToolPosition(outil, tool_bank);
        if (tool_pos_int == 0)
        {
            tool_pos = "00";
        }
        else
        {
            tool_pos = formatNumber(((tool_pos_int) + 1), 2);
        }
        if (tool_pos != "-98")
        {
            commonParams.emplacementOutil = tool_pos;
            commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation, 4);
            commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance * 100), 3);
            nouveau_fraisurage.angleFraise = formatNumber(outil.angle, 3); // Modifier le chiffre brut avec bonne valeur
            nouveau_fraisurage.diametreExterieur = formatNumber((operation.rayon * 2) * 10, 3);
            nouveau_fraisurage.commonParams = commonParams;
            std::string fraisurageCommand = generateFraisurageCommand(nouveau_fraisurage);
            // Swap the positions in both the JSON file and the array
            std::swap(tool_bank[0], tool_bank[tool_pos_int]);
            // Update the JSON file with the swapped positions
            updateJsonFile(tool_bank);
            return fraisurageCommand;
        }
        else
        {
            // std::cout << "No Tool Found!" << std::endl;
        }
        break;
    case 2: // Taraudage (OP. 2)
    case 3: // Taraudage (OP. 3)
        outil = selectTool(operation.rayon, "T", tool_bank);
        tool_pos_int = findToolPosition(outil, tool_bank);
        if (tool_pos_int == 0)
        {
            tool_pos = "00";
        }
        else
        {
            tool_pos = formatNumber(((tool_pos_int) + 1), 2);
        }
        if (tool_pos != "-98")
        {
            commonParams.emplacementOutil = tool_pos;
            commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation, 4);
            commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance * 100), 3);
            nouveau_taraudage.pasOutil = formatNumber((operation.z * 100), 3);
            nouveau_taraudage.commonParams = commonParams;
            std::string taraudageCommand = generateTaraudageCommand(nouveau_taraudage);
            // Swap the positions in both the JSON file and the array
            std::swap(tool_bank[0], tool_bank[tool_pos_int]);
            // Update the JSON file with the swapped positions
            updateJsonFile(tool_bank);
            return taraudageCommand;
        }
        else
        {
            // std::cout << "No Tool Found!" << std::endl;
        }

        break;
    case 5: // Lamage (OP. 5)
        outil = selectTool(operation.rayon, "S", tool_bank);
        tool_pos_int = findToolPosition(outil, tool_bank);
        if (tool_pos_int == 0)
        {
            tool_pos = "00";
        }
        else
        {
            tool_pos = formatNumber(((tool_pos_int) + 1), 2);
        }
        if (tool_pos != "-98")
        {
            commonParams.emplacementOutil = tool_pos;
            commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation, 4);
            commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance * 100), 3);
            nouveau_lamage.longueurPiloteOutil = formatNumber(outil.longueur_pilote, 3); // Modifier le chiffre brut avec bonne valeur
            nouveau_lamage.profondeur = formatNumber((operation.z * 100), 3);
            nouveau_lamage.commonParams = commonParams;
            std::string lamageCommand = generateLamageCommand(nouveau_lamage);
            // Swap the positions in both the JSON file and the array
            std::swap(tool_bank[0], tool_bank[tool_pos_int]);
            // Update the JSON file with the swapped positions
            updateJsonFile(tool_bank);
            return lamageCommand;
        }
        else
        {
            // std::cout << "No Tool Found!" << std::endl;
        }
        break;
    default:
        // std::cout << "Unknown operation type: " << opType;
        break;
    }

    return "ERR";
}

// ---------------- END GENERATION DE LA TRAME ROBOT ---------------------------------- //

// MAIN-----------------------------------------------------------

SymValueVariant driller_frames_execute(std::string filename, int operational_mode)
{

    std::vector<SymValue> Transformation_Repere_Tole, Transformation_Repere_Robot, pre_final_values, raw_holes_all;
    std::vector<SymValueGroup> final_values_groups;
    std::vector<SymValueSections> final_values_sections;
    Outil currentTool = {};
    std::vector<std::string> liste_trames;

    // Création d'une structure de coordonnées pour le repère de la tôle
    Coordinates repere_tole;
    repere_tole.x = 1086.18;
    repere_tole.y = 1981.08;
    repere_tole.theta = 180.801743;

    // Affichage banque d'outils
    // for (const auto& tool : toolBank_json) {
    //    std::cout << "Tool: " << tool.classe << ", Diameter: " << tool.diametre << std::endl;
    //}

    // std::vector<Outil> toolMagasine = readToolBank("tool_bank.json");

    // // Check s'il y a au moins 20 outils dans la banque
    // if (toolBank.size() >= 20)
    // {
    //     // Recupere les 20 premiers outils dans la banque
    //     toolMagasine.assign(toolBank.begin(), toolBank.begin() + 20);
    // }
    // else
    // {
    //     // Pas assez d'outils dans la banque d'outils
    //     std::cerr << "Error: Not enough tools in the toolBank." << std::endl;
    //     return {}; // Return an empty vector
    // }

    // Ouverture du fichier en lecture
    std::ifstream file(filename);
    if (!file)
    {
        // Affiche un message d'erreur si l'ouverture du fichier échoue
        std::cerr << "Error opening file: " << filename << std::endl;
        return {}; // Return an empty vector
    }

    epaisseur_tole = getThickness(filename);
    if (epaisseur_tole == "ERR")
    {
        std::cerr << "Epaisseur not found or an error occurred." << std::endl;
    }

    // Lecture du contenu complet du fichier dans une chaîne de caractères
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Extraction des chemins et des coordonnées à partir du contenu du fichier
    std::vector<PathCoordinates> pathCoordinatesList = extractPathAndCoordinates(fileContent);

    // Vérification si la liste des coordonnées est vide
    if (pathCoordinatesList.empty())
    {
        // std::cout << "Aucun contenu dans la section 200." << std::endl;
        return {}; // Return an empty vector
    }

    // Affichage des chemins et des coordonnées
    // printPathCoordinates(pathCoordinatesList);
    // for (auto &pathCoordinatesTest : pathCoordinatesList){
    //     std::cout << "Element: "<< pathCoordinatesTest.path << std::endl;
    // }

    // int count = 0;
    // std::cout << pathCoordinatesList.size() << std::endl;
    //  Boucle pour gérer les fichiers .sym
    for (auto &pathCoordinates : pathCoordinatesList)
    {
        // std::cout << "ELEMENT : "<<  pathCoordinates.path << std::endl;
        //  Vérifie si le chemin se termine par ".sym"
        // if (pathCoordinates.path.substr(pathCoordinates.path.size() - 4) == ".sym")
        //{

        // std::cout <<  count++ << std::endl;
        //  Extraction des données brutes du fichier .sym
        std::vector<SymValue> raw_holes_data = extractSymData(pathCoordinates.path);
        // auto insertion_point = raw_holes_all.end();

        // Inserting vector1 into vector2 without erasing existing data
        // raw_holes_all.insert(insertion_point, raw_holes_data.begin(), raw_holes_data.end());
        // raw_holes_all.push_back(raw_holes_data);
        // for (SymValue hole : raw_holes_data)
        // {
        //     std::cout << "Type : " << hole.type << " | Diametre : " << hole.rayon*2 << std::endl;
        // }

        // Vérifie si les données brutes ne sont pas vides
        if (!raw_holes_data.empty())
        {

            // Transformation 1 : Transformation des coordonnées dans le repère de la tôle
            for (const Coordinates &coordinate : pathCoordinates.coordinates)
            {
                std::cout << "REF = X: " << coordinate.x << "| Y: " << coordinate.y << std::endl;

                for (const SymValue &value : raw_holes_data)
                {
                    std::cout << "BRUT = X: " << value.x << "| Y: " << value.y << std::endl;

                    // Applique la transformation dans le repère de la tôle
                    SymValue transformedValue = transformation_repere_rad(value, coordinate);
                    std::cout << "REPERE TOLE = X: " << transformedValue.x << "| Y: " << transformedValue.y << std::endl;
                    // Ajoute les coordonnées transformées au vecteur Transformation_Repere_Tole
                    Transformation_Repere_Tole.push_back(transformedValue);
                }
            }

            Transformation_Repere_Robot.clear();
            // Transformation 2 : Transformation des coordonnées dans le repère du robot
            for (const SymValue &value : Transformation_Repere_Tole)
            {
                // Applique la transformation dans le repère du robot
                SymValue transformedValue = transformation_repere_rad(value, repere_tole);
                std::cout << "REPERE ROBOT = X: " << transformedValue.x << "| Y: " << transformedValue.y << std::endl;

                // Ajoute les coordonnées transformées au vecteur Transformation_Repere_Robot
                Transformation_Repere_Robot.push_back(transformedValue);
            }
            //
            //Transformation_Repere_Tole.clear();
            // Tri des données transformées dans le repère robot
            switch (operational_mode)
            {
            case 0:
                final_values_groups = sortAndGroupByType(Transformation_Repere_Robot, customTypeOrder);
                for (SymValueGroup &group : final_values_groups)
                {
                    std::sort(group.values.begin(), group.values.end(), sortByRayonAndDistance);
                }

                // return final_values_groups;
                break;
            case 1:
                final_values_sections = sortAndGroupBySection(Transformation_Repere_Robot, customTypeOrder);

                for (SymValueSections &section : final_values_sections)
                {
                    for (SymValueGroup &group : section.groups)
                    {
                        std::sort(group.values.begin(), group.values.end(), sortByRayonAndDistance);
                    }
                }
                // return final_values_sections;
                break;
            default:
                std::cout << "OPERATIONAL MODE ERROR" << std::endl;
                return {};
            }

            // Affichage des données transformées (test)
            // for (SymValueGroup& group : final_values) {
            // printStructure(group.values);
            //}
        }

        //}
    }

    if (!final_values_groups.empty())
    {
        return final_values_groups;
    }
    else
    {
        return final_values_sections;
    }
    // return liste_trames; // Quitte le programme avec un code de succès
    // return final_values;
    return {};
}

void extract_images(std::string filename)
{
    // Initialize Python
    Py_Initialize();

    // Import required Python modules
    PyRun_SimpleString("import base64");
    PyRun_SimpleString("from PIL import Image");
    PyRun_SimpleString("from io import BytesIO");

    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    // Convert current time to a string format
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", std::localtime(&current_time));
    std::string timestamp(buffer);

    int img_count = 0;

    // Create a folder with the current timestamp
    std::string folderPath = "images/" + timestamp;
    std::filesystem::create_directory(folderPath);

    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        // Print an error message if the file fails to open
        std::cerr << "Error opening file: " << std::endl;
        return;
    }
    // Read the entire file content into a string
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Extract all image sections from the file content
    std::vector<std::string> imageSections = extractAllImageSections(fileContent);

    // Iterate through each extracted image section
    for (const auto &imageSection : imageSections)
    {
        // Extract the CDATA content from the current image section
        std::string imageData = extractImageData(imageSection);

        if (!imageData.empty())
        {
            // Clean the CDATA content (remove white spaces and empty characters)
            std::string imageDataCleaned = cleanString(imageData);

            // Prepare Python code to decode base64 and save the image
            std::string pythonCode = "data = '''" + imageDataCleaned + "'''\n"
                                                                       "im = Image.open(BytesIO(base64.b64decode(data)))\n"
                                                                       "im.save('" +
                                     folderPath + "/image-" + std::to_string(img_count) + ".png', 'PNG')\n";

            // Execute Python code
            PyRun_SimpleString(pythonCode.c_str());

            img_count++;

            // std::cout << "Image Data Length: " << imageDataCleaned.length() << std::endl;
            // std::cout << "Image Data: " << imageDataCleaned << std::endl;
        }
        else
        {
            std::cout << "No CDATA content found in the Image section." << std::endl;
        }
    }
    // Finalize Python
    Py_Finalize();
}