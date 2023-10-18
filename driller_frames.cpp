//BUILD : g++ -std=c++11 -o driller_frames driller_frames.cpp
//EXECUTE : ./driller_frames [filename.drg]

#include <iostream>       // Inclut la bibliothèque d'entrée/sortie
#include <fstream>        // Inclut la bibliothèque pour la lecture/écriture de fichiers
#include <string>         // Inclut la bibliothèque pour manipuler les chaînes de caractères
#include <vector>         // Inclut la bibliothèque pour utiliser des vecteurs
#include <sstream>        // Inclut la bibliothèque pour manipuler des flux de chaînes de caractères
#include <algorithm>      // Inclut la bibliothèque pour utiliser la fonction std::sort
#include <cstdlib>
//Bibliotheques Windows et Linux
#include <iomanip>
#include <limits>
#include <unordered_map>
#include <climits>
#include <cmath>

#include "driller_frames.h" // Inclut le header avec les prototypes des fonctions de generation de trames




//BANQUE D'OUTILS
std::vector<Outil> toolBank = {
    // Perçage (Classe D)
    { 800.0, 5.0, 100.0, "D", 5.0, 60.0, 45.0, 0 },
    { 800.0, 5.0, 100.0, "D", 6.0, 60.0, 45.0, 0 },
    { 800.0, 5.0, 100.0, "D", 7.0, 60.0, 45.0, 0 },
    { 800.0, 5.0, 100.0, "D", 8.0, 60.0, 45.0, 0 },
    { 800.0, 5.0, 100.0, "D", 8.5, 60.0, 45.0, 0 },
    { 1500.0, 8.0, 200.0, "D", 9.0, 75.0, 90.0, 0 },
    { 1500.0, 8.0, 200.0, "D", 10.0, 75.0, 90.0, 0 },
    { 1200.0, 6.0, 200.0, "D", 12.0, 75.0, 60.0, 0 },
    { 1200.0, 6.0, 200.0, "D", 14.0, 75.0, 60.0, 0 },
    // Fraisurage (Classe C)
    { 1000.0, 5.0, 100.0, "C", 10.0, 50.0, 90.0, 0, 0.0 },
    { 1500.0, 7.0, 200.0, "C", 20.0, 75.0, 60.0, 0, 10.0 },
    // Taraudage (Classe T)
    { 800.0, 5.0, 125.0, "T", 6.0, 60.0, 45.0, 0 },
    { 1000.0, 6.0, 200.0, "T", 8.0, 75.0, 60.0, 0 },
    { 1500.0, 8.0, 200.0, "T", 10.0, 75.0, 90.0, 0 },
    // Lamage (Classe S)
    { 800.0, 5.0, 100.0, "S", 6.0, 60.0, 45.0, 0 },
    { 1000.0, 6.0, 200.0, "S", 8.0, 75.0, 60.0, 0 },
    { 1200.0, 8.0, 200.0, "S", 10.0, 75.0, 90.0, 0 },
    { 800.0, 5.0, 100.0, "S", 7.0, 60.0, 45.0, 0 },
    { 1000.0, 6.0, 200.0, "S", 9.0, 75.0, 60.0, 0 },
    { 1200.0, 8.0, 200.0, "S", 11.0, 75.0, 90.0, 0 },
};



static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(BYTE c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}


// Function to clean a string by removing white spaces and empty characters
std::string cleanString(const std::string& input) {
    std::string result = input;

    // Remove white spaces (spaces, tabs, newlines, etc.)
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());

    // Remove empty characters (null characters, etc.)
    result.erase(std::remove_if(result.begin(), result.end(), ::iscntrl), result.end());

    return result;
}


// Function to extract the first <Image> section with image-type="png"
std::string extractImageSection(const std::string& content) {
    std::string imageSection;

    // Find the beginning of the <Image> section with image-type="png"
    size_t imageStart = content.find("<Image image-type=\"png\"");
    if (imageStart != std::string::npos) {
        // Find the end of the <Image> section
        size_t imageEnd = content.find("</Image>", imageStart);
        if (imageEnd != std::string::npos) {
            // Extract the <Image> section including attributes and CDATA content
            imageSection = content.substr(imageStart, imageEnd - imageStart + 9); // +9 to include </Image>
        }
    }

    return imageSection;
}

// Function to extract the CDATA content from an <Image> section
std::string extractImageData(const std::string& imageSection) {
    std::string imageData;

    // Find the beginning of the CDATA section
    size_t cdataStart = imageSection.find("<![CDATA[");
    if (cdataStart != std::string::npos) {
        // Find the end of the CDATA section
        size_t cdataEnd = imageSection.find("]]>", cdataStart);
        if (cdataEnd != std::string::npos) {
            // Extract the CDATA content
            imageData = imageSection.substr(cdataStart + 9, cdataEnd - (cdataStart + 9)); // +9 to skip "<![CDATA["
        }
    }

    return imageData;
}

std::vector<BYTE> base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  BYTE char_array_4[4], char_array_3[3];
  std::vector<BYTE> ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
          ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  //std::cout << is_base64(encoded_string[in_]) << std::endl;

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
  }

  return ret;
}

// Function to save image data as a PNG file
void saveImageToPNG(const std::vector<unsigned char>& imageData, unsigned int width, unsigned int height, const std::string& filename) {
    // Define the PNG file header
    std::vector<unsigned char> pngHeader = {
        137, 80, 78, 71, 13, 10, 26, 10, // PNG signature
        0, 0, 0, 13,                    // IHDR chunk length
        73, 72, 68, 82,                 // "IHDR" chunk type
        static_cast<unsigned char>(width >> 24), static_cast<unsigned char>(width >> 16),
        static_cast<unsigned char>(width >> 8), static_cast<unsigned char>(width), // Image width
        static_cast<unsigned char>(height >> 24), static_cast<unsigned char>(height >> 16),
        static_cast<unsigned char>(height >> 8), static_cast<unsigned char>(height), // Image height
        8,                              // Bit depth (8 bits per channel)
        6                               // Color type (RGB with alpha)
    };

    // Combine the header and image data
    std::vector<unsigned char> pngData;
    pngData.insert(pngData.end(), pngHeader.begin(), pngHeader.end());
    pngData.insert(pngData.end(), imageData.begin(), imageData.end());

    // Write the PNG data to the file
    std::ofstream outputFile(filename, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error creating output file!" << std::endl;
        return;
    }
    outputFile.write(reinterpret_cast<char*>(pngData.data()), pngData.size());
    std::cout << "Image saved to '" << filename << "'" << std::endl;
}


// Function to select a tool from the tool bank based on rayon and classe
Outil selectTool(double rayon, const std::string& classe) {
    Outil selectedTool;
    for (const Outil& tool : toolBank) {
        if (tool.classe == classe) {
            if (classe == "C") {
                // For class "C", choose a tool if rayon <= tool.diametre / 2
                if (rayon <= tool.diametre / 2 && rayon >= tool.diametre_min / 2) {
                    selectedTool = tool;
                    break; // Stop after selecting the first suitable tool
                }
            } else {
                // For other classes, choose a tool if rayon is exactly equal to tool.diametre / 2
                if (rayon == tool.diametre / 2) {
                    selectedTool = tool;
                    break; // Stop after selecting the first suitable tool
                }
            }
        }
    }
    return selectedTool;
}

// Function to find the position of a tool in the toolBank vector
int findToolPosition(const Outil& tool) {
    for (int i = 0; i < toolBank.size(); ++i) {
        if (toolBank[i].vitesse_rotation == tool.vitesse_rotation &&
           toolBank[i].vitesse_avance == tool.vitesse_avance &&
           toolBank[i].pas_outil == tool.pas_outil &&
           toolBank[i].classe == tool.classe &&
           toolBank[i].diametre == tool.diametre &&
           toolBank[i].longueur_pilote == tool.longueur_pilote &&
           toolBank[i].angle == tool.angle) {
            return i; // Found the tool, return its position
        }
    }
    return -1; // Tool not found in the vector
}




// Fonction utilitaire pour extraire le nom du fichier du chemin complet
std::string getFilenameFromPath(const std::string& path) {
    // Trouve la dernière occurrence de '/' ou '\' dans le chemin
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        // Retourne la partie du chemin après la dernière occurrence
        return path.substr(lastSlash + 1);
    }
    // Si aucun '/' ou '\' n'est trouvé, retourne le chemin complet comme nom de fichier
    return path;
}


std::string formatNumber(double value, int digits){
    // Format the value as a 3-digit string with leading zeros
    std::stringstream ss;
    ss << std::setw(digits) << std::setfill('0') << value;
    return ss.str();
}

// Function to retrieve the 'thickness' attribute from a .drg file as a 3-digit string
std::string getThickness(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            size_t pos = line.find("name=\"Epaisseur\"");
            if (pos != std::string::npos) {
                // Found the line containing "name=\"Epaisseur\""
                pos = line.find("value=\"");
                if (pos != std::string::npos) {
                    // Found the line containing "value=\""
                    pos += 7; // Move to the beginning of the actual value
                    size_t endPos = line.find("\"", pos);
                    if (endPos != std::string::npos) {
                        // Found the closing quote for the value
                        std::string valueStr = line.substr(pos, endPos - pos);
                        try {
                            // Convert the value to an integer (assuming it's in cm)
                            int valueCm = std::stoi(valueStr);
                            // Convert cm to mm (1 cm = 10 mm)
                            int valueMm = valueCm * 10;
                            // Format the value as a 3-digit string with leading zeros
                            std::stringstream ss;
                            ss << std::setw(3) << std::setfill('0') << valueMm;
                            return ss.str();
                        } catch (const std::exception& e) {
                            // Handle conversion error (e.g., value is not an integer)
                            std::cerr << "Error parsing 'value' attribute: " << e.what() << std::endl;
                            return "ERR"; // Error code as a string
                        }
                    }
                }
            }
        }
        file.close();
    } else {
        std::cerr << "Error opening file: " << filePath << std::endl;
    }

    return "N/A"; // Value not found or an error occurred, return "N/A"
}

// Fonction pour extraire les chemins et les coordonnées à partir d'une chaîne de contenu
std::vector<PathCoordinates> extractPathAndCoordinates(const std::string& content) {
    // Initialise une liste de PathCoordinates vide
    std::vector<PathCoordinates> pathCoordinatesList;

    // Recherche le début de l'attribut num=200 dans le contenu
    size_t attr200Start = content.find("<Attr num=\"200\" name=\"AKEROS\"");
    if (attr200Start == std::string::npos) {
        // Affiche un message d'erreur si l'attribut num=200 n'est pas trouvé
        std::cerr << "Attribute num=200 not found in the file." << std::endl;
        return pathCoordinatesList;
    }

    // Recherche le début de la section "value" dans l'attribut num=200
    size_t valueStart = content.find("value=\"", attr200Start);
    if (valueStart == std::string::npos) {
        // Affiche un message d'erreur si la section "value" n'est pas trouvée
        std::cerr << "Value section not found for Attribute num=200." << std::endl;
        return pathCoordinatesList;
    }

    // Déplace le curseur vers le début des données de la section "value"
    valueStart += 7; // 7 caractères pour atteindre le début des données

    // Recherche la fin de la section "value"
    size_t valueEnd = content.find("\"", valueStart);
    if (valueEnd == std::string::npos) {
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
    while ((pos = valueSection.find(delimiter)) != std::string::npos) {
        // Extrait un élément de la section "value"
        std::string section = valueSection.substr(0, pos);
        if (section.substr(0, 2) == "\\\\") {
            // Si l'élément commence par "\\"
            PathCoordinates pathCoordinates;
            pathCoordinates.path = section;
            pathCoordinatesList.push_back(pathCoordinates);
        } else {
            // Si l'élément n'est pas une chaîne commençant par "\\"
            // Convertit la chaîne de coordonnées en une structure Coordinates
            std::istringstream iss(section);
            double x, y, theta;
            char coordDelimiter = '|';

            // Ignore le caractère '[' initial
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');

            // Extrait les valeurs x, y et theta
            if (iss >> x >> coordDelimiter >> y >> coordDelimiter >> theta) {
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
    if (!valueSection.empty() && valueSection.substr(0, 2) != "\\\\") {
        // Convertit la chaîne de coordonnées en une structure Coordinates
        std::istringstream iss(valueSection);
        double x, y, theta;
        char coordDelimiter = '|';

        // Ignore le caractère '[' initial
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');

        // Extrait les valeurs x, y et theta
        if (iss >> x >> coordDelimiter >> y >> coordDelimiter >> theta) {
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
std::string extractSym200(const std::string& symFileContent) {
    // Recherche la balise d'ouverture de la section "200"
    std::string openingTag = "<Attr num=\"200\"";
    size_t openingPos = symFileContent.find(openingTag);
    if (openingPos == std::string::npos) {
        // Si la section "200" n'est pas trouvée, retourne une chaîne vide
        return "";
    }

    // Recherche la balise de fermeture de la section "200"
    size_t closingPos = symFileContent.find("</Attr>", openingPos);
    if (closingPos == std::string::npos) {
        // Si la section "200" n'est pas terminée correctement, retourne une chaîne vide
        return "";
    }

    // Extrait le contenu de la section "200" avec les balises
    std::string section200 = symFileContent.substr(openingPos, closingPos - openingPos + 8);

    return section200;
}

// Fonction pour afficher les chemins et les coordonnées
void printPathCoordinates(std::vector<PathCoordinates>& pathCoordinatesList) {
    int pathCount = 1;
    for (auto& pathCoordinates : pathCoordinatesList) {
        // Supprime les caractères de nouvelle ligne de pathCoordinates.path
        size_t pos;
        while ((pos = pathCoordinates.path.find('\n')) != std::string::npos) {
            pathCoordinates.path.erase(pos, 1);
        }

        // Affiche le numéro de pièce et le chemin
        std::cout << "Pièce " << pathCount << " : " << pathCoordinates.path << std::endl;
        std::cout << "Coordonnées des exemplaires (pièce " << pathCount << ") : " << std::endl;

        // Itère sur les coordonnées de pathCoordinates.coordinates (un vecteur de Coordinates)
        for (const Coordinates& coord : pathCoordinates.coordinates) {
            std::cout << "x: " << coord.x << ", y: " << coord.y << ", theta: " << coord.theta << std::endl;
        }

        pathCount++;
    }
}

// Fonction pour extraire les données SymValue à partir de la section "value"
std::vector<SymValue> parseValueSection(const std::string& valueSection) {
    std::vector<SymValue> symValues;

    // Recherche la position de l'attribut "value"
    size_t pos = valueSection.find("value=\"");
    if (pos == std::string::npos) {
        // Affiche un message d'erreur si l'attribut "value" n'est pas trouvé
        std::cerr << "Aucune opération à effectuer." << std::endl;
        return symValues; // Retourne un vecteur vide en cas d'erreur
    }

    // Avance la position au début des valeurs
    pos += 7; // Avance de 7 caractères pour atteindre le début des valeurs

    // Recherche la position de la fermeture des guillemets
    size_t endPos = valueSection.find("\"", pos);
    if (endPos == std::string::npos) {
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
    while (std::getline(valueStream, token, '|')) {
        // Initialise un flux de chaînes de caractères à partir de l'élément
        std::istringstream tokenStream(token);
        SymValue symValue;
        char separator;

        // Lit les valeurs de l'élément et les stocke dans la structure SymValue
        tokenStream >> symValue.x >> separator >> symValue.y >> separator >> symValue.z >> separator
                    >> symValue.type >> separator >> symValue.couleur >> separator >> symValue.rayon;

        if (tokenStream.fail()) {
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
std::vector<SymValue> extractSymData(const std::string& symFilePath) {
    // Initialise un vecteur de SymValue pour stocker les données extraites
    std::vector<SymValue> parsedValues;

    // Extrait le nom de fichier du chemin complet
    std::string filename = getFilenameFromPath(symFilePath);

    // Ouvre le fichier .sym
    std::ifstream symFile(filename);
    if (!symFile) {
        // Affiche un message d'erreur en cas d'échec de l'ouverture du fichier
        std::cerr << "Error opening .sym file: " << symFilePath << std::endl;
    }

    // Lit le contenu complet du fichier .sym dans une chaîne de caractères
    std::string symFileContent((std::istreambuf_iterator<char>(symFile)), std::istreambuf_iterator<char>());

    // Extrait le contenu de la section "200"
    std::string section200Content = extractSym200(symFileContent);

    // Vérifie si la section "200" est présente
    if (!section200Content.empty()) {
        // Affiche un message indiquant les opérations à effectuer pour chaque exemplaire de la pièce
        std::cout << "Operations à effectuer pour chaque exemplaire de la pièce -> " << filename << std::endl;

        // Appelle la fonction pour analyser la section "value" et stocker les données dans parsedValues
        parsedValues = parseValueSection(section200Content);

        // Boucle for utilisée pour afficher les valeurs (commentée car elle est en cours de développement)
        //for (const SymValue& value : parsedValues) {
        //    std::cout << "[" + std::to_string(value.x)+ "," + std::to_string(value.y) + "," + std::to_string(value.z) + "," + std::to_string(value.type) + "," + std::to_string(value.couleur) + "," + std::to_string(value.rayon) + "]" << std::endl;
        //}
    } else {
        // Affiche un message d'erreur si la section "200" n'est pas trouvée
        std::cerr << "Section 200 not found in .sym file: " << symFilePath << std::endl;
    }

    // Retourne le vecteur de SymValues
    return parsedValues;
}

std::vector<SymValueGroup> sortAndGroupByType(const std::vector<SymValue>& inputValues, const std::vector<int>& typeOrder) {
    // Initialize a vector of SymValueGroup
    std::vector<SymValueGroup> groupedValues;

    // Create a map to store the custom order for each type
    std::unordered_map<int, int> typeOrderMap;
    for (size_t i = 0; i < typeOrder.size(); ++i) {
        typeOrderMap[typeOrder[i]] = static_cast<int>(i);
    }

    // Sort the inputValues by type using the custom order
    std::vector<SymValue> sortedValues = inputValues;
    std::sort(sortedValues.begin(), sortedValues.end(), [&](const SymValue& a, const SymValue& b) {
        int typeAOrder = typeOrderMap.find(a.type) != typeOrderMap.end() ? typeOrderMap[a.type] : INT_MAX;
        int typeBOrder = typeOrderMap.find(b.type) != typeOrderMap.end() ? typeOrderMap[b.type] : INT_MAX;

        if (typeAOrder != typeBOrder) {
            return typeAOrder < typeBOrder;
        }

        // If types have the same order, sort by type
        return a.type < b.type;
    });

    // Group the sorted values by type
    for (const SymValue& value : sortedValues) {
        int valueType = value.type;

        // Try to find a group with the same type
        auto groupIt = std::find_if(groupedValues.begin(), groupedValues.end(), [&](const SymValueGroup& group) {
            return group.type == valueType;
        });

        if (groupIt != groupedValues.end()) {
            // Add the value to an existing group with the same type
            groupIt->values.push_back(value);
        } else {
            // Create a new group and add the value to it
            SymValueGroup newGroup;
            newGroup.type = valueType;
            newGroup.values.push_back(value);
            groupedValues.push_back(newGroup);
        }
    }

    return groupedValues;
}



// Fonction de tri des données transformées
bool sortByRayonAndDistance(const SymValue& a, const SymValue& b) {
    // Ensuite, compare par rayon
    if (a.rayon != b.rayon) {
        return a.rayon < b.rayon;
    }

    // Enfin, compare par distance euclidienne
    double distanceA = std::sqrt(a.x * a.x + a.y * a.y);
    double distanceB = std::sqrt(b.x * b.x + b.y * b.y);

    return distanceA < distanceB;
}



// Fonction pour afficher les données SymValue
void printStructure(const std::vector<SymValue>& values) {
    for (const SymValue& value : values) {
        std::cout << "[" << value.x << "; " << value.y << "; " << value.z << "; "
                  << value.type << "; " << value.couleur << "; " << value.rayon << "]" << std::endl;
    }
}

// FONCTIONS DE TRANSFORMATION (1 ET 2)
// ANGLE -> Theta des pièces
// POINT -> Coordonnées de l'opération de chaque exemplaire de la pièce
// ORIGINE -> Coordonnées de la pièce dans la tôle

// Resultat.X := (COS (Angle/57.2958) * Point.X) - (SIN (Angle/57.2958) * Point.Y) + Origine.x;
// Resultat.Y := (SIN (Angle/57.2958) * Point.X) + (COS (Angle/57.2958) * Point.Y) + Origine.Y;

// Fonction de transformation des coordonnées dans le repère de la tôle (en radians)
SymValue transformation_repere_rad(const SymValue& point, const Coordinates& repere) {
    SymValue transformed = point;
    transformed.x = (cos(repere.theta / 57.2958) * point.x) - (sin(repere.theta / 57.2958) * point.y) + repere.x;
    transformed.y = (sin(repere.theta / 57.2958) * point.x) + (cos(repere.theta / 57.2958) * point.y) + repere.y;
    return transformed;
}



// ---------------- GENERATION DE LA TRAME ROBOT ---------------------------------- //


// Trame Percage (OP. 1)
std::string generatePercageCommand(const PercageParams& params) {
    return "MS A D " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.commonParams.vitesseAvanceOutil;
}

// Trame Fraisurage (OP. 4)
std::string generateFraisurageCommand(const FraisurageParams& params) {
    return "MS A C " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.commonParams.vitesseAvanceOutil +
           " " + params.angleFraise + " " + params.diametreExterieur;
}

// Trame Taraudage (OP. 2 et 3)
std::string generateTaraudageCommand(const TaraudageParams& params) {
    return "MS A T " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.pasOutil;
}

// Trame Lamage (OP. 5)
std::string generateLamageCommand(const LamageParams& params) {
    return "MS A S " + params.commonParams.coordX + " " + params.commonParams.coordY + " " +
           params.commonParams.epaisseurTole + " " + params.commonParams.emplacementOutil + " " +
           params.commonParams.vitesseRotationOutil + " " + params.commonParams.vitesseAvanceOutil +
           " " + params.profondeur + " " + params.longueurPiloteOutil;
}


// Function to generate commands for all elements in final_values
void generateCommands(const std::vector<SymValueGroup>& final_values, const std::string& epaisseur_tole) {
    PercageParams nouveau_percage;
    FraisurageParams nouveau_fraisurage;
    TaraudageParams nouveau_taraudage;
    LamageParams nouveau_lamage;
    Outil outil;
    std::string tool_pos;
    
    for (const SymValueGroup& group : final_values) {
        // Assuming the group type corresponds to the operation type (OP number)
        int opType = group.type;
        for (const SymValue& operation : group.values) { 
            
        
        // Common parameters
        CommonParams commonParams;
        commonParams.coordX = formatNumber((operation.x * 100), 6); // Convert to mm
        commonParams.coordY = formatNumber((operation.y * 100), 6); // Convert to mm
        commonParams.epaisseurTole = epaisseur_tole;

        // Generate commands based on operation type
        switch (opType) {
            case 1: // Percage (OP. 1)
                outil = selectTool(operation.rayon, "D");
                tool_pos = formatNumber(((findToolPosition(outil)) + 1), 2) ;
                if (tool_pos != "00") {
                commonParams.emplacementOutil = tool_pos;
                commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation,4);
                commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance*100), 3);
                nouveau_percage.commonParams = commonParams;
                std::cout << generatePercageCommand(nouveau_percage);
                } else {
                std::cout << "No Tool Found!";
                }
                break;
            case 4: // Fraisurage (OP. 4)
                outil = selectTool(operation.rayon, "C");
                tool_pos = formatNumber(((findToolPosition(outil)) + 1), 2) ;
                if (tool_pos != "00") {
                commonParams.emplacementOutil = tool_pos;
                commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation,4);
                commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance*100), 3);
                nouveau_fraisurage.angleFraise = formatNumber(outil.angle, 3); // Modifier le chiffre brut avec bonne valeur
                nouveau_fraisurage.diametreExterieur = formatNumber((operation.rayon*2)*10, 3);
                nouveau_fraisurage.commonParams = commonParams;
                std::cout << generateFraisurageCommand(nouveau_fraisurage);
                } else {
                std::cout << "No Tool Found!";
                }
                break;
            case 2: // Taraudage (OP. 2)
            case 3: // Taraudage (OP. 3)
                outil = selectTool(operation.rayon, "T");
                tool_pos = formatNumber(((findToolPosition(outil)) + 1), 2) ;
                if (tool_pos != "00") {
                commonParams.emplacementOutil = tool_pos;
                commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation,4);
                commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance*100), 3);
                nouveau_taraudage.pasOutil = formatNumber((operation.z * 100), 3);
                nouveau_taraudage.commonParams = commonParams;
                std::cout << generateTaraudageCommand(nouveau_taraudage);
                } else {
                std::cout << "No Tool Found!";
                }


                break;
            case 5: // Lamage (OP. 5)
            outil = selectTool(operation.rayon, "S");
            tool_pos = formatNumber(((findToolPosition(outil)) + 1), 2) ;
                if (tool_pos != "00") {
                commonParams.emplacementOutil = tool_pos;
                commonParams.vitesseRotationOutil = formatNumber(outil.vitesse_rotation,4);
                commonParams.vitesseAvanceOutil = formatNumber((outil.vitesse_avance*100), 3);
                nouveau_lamage.longueurPiloteOutil = formatNumber(outil.longueur_pilote,3);  // Modifier le chiffre brut avec bonne valeur
                nouveau_lamage.profondeur = formatNumber((operation.z * 100), 3);
                nouveau_lamage.commonParams = commonParams;
                std::cout << generateLamageCommand(nouveau_lamage);
                } else {
                std::cout << "No Tool Found!";
                }
            break;
            default:
                std::cout << "Unknown operation type: " << opType;
                break;
        }

        std::cout << std::endl;
    }
    }
}



// ---------------- END GENERATION DE LA TRAME ROBOT ---------------------------------- //


// MAIN-----------------------------------------------------------
/*
int main(int argc, char* argv[]) {
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

    return 0; // Quitte le programme avec un code de succès
}
*/