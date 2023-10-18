#ifndef DRILLER_FRAMES_H
#define DRILLER_FRAMES_H



// STRUCTURES

// Structure to store image information
struct ImageInfo {
    int size;
    std::string data;
};


// Structure pour stocker les coordonnées (x, y, theta)
struct Coordinates {
    double x;
    double y;
    double theta;
};

// Structure pour stocker le chemin et les coordonnées
struct PathCoordinates {
    std::string path;
    std::vector<Coordinates> coordinates;
};

// Structure pour stocker les valeurs symétriques
struct SymValue {
    double x;
    double y;
    double z;
    int type;
    int couleur;
    double rayon;
};

// Structure to represent a group of SymValue objects with the same type
struct SymValueGroup {
    int type;
    std::vector<SymValue> values;
};


// Structure pour stocker les repères de la tôle
struct RepereTole {
    double x;
    double y;
    double theta;
};



// Structure for common parameters
struct CommonParams {
    std::string coordX;                 // Coordonnee X
    std::string coordY;                 // Coordonnee Y
    std::string epaisseurTole;         // Epaisseur de la tole
    std::string emplacementOutil = "00";      // Emplacement de l'outil
    std::string vitesseRotationOutil;  // Vitesse de rotation de l'outil
    std::string vitesseAvanceOutil;    // Vitesse d'avance de l'outil
};

// Structure for Percage (OP. 1)
struct PercageParams {
    CommonParams commonParams; // Common parameters
};

// Structure for Fraisurage (OP. 4)
struct FraisurageParams {
    CommonParams commonParams;    // Common parameters
    std::string angleFraise;       // Angle de la fraise
    std::string diametreExterieur; // Diametre exterieur du chanfrein
};

// Structure for Taraudage (OP. 2 et 3)
struct TaraudageParams {
    CommonParams commonParams; // Common parameters
    std::string pasOutil;      // Pas de l'outil
};

// Structure for Lamage (OP. 5)
struct LamageParams {
    CommonParams commonParams;      // Common parameters
    std::string profondeur;         // Profondeur
    std::string longueurPiloteOutil; // Longueur du pilote de l'outil
};

struct Outil {
    double vitesse_rotation;
    double vitesse_avance;
    double pas_outil;
    std::string classe;
    double diametre;
    double longueur_pilote;
    double angle;
    int nombre_cycles;
    double diametre_min;
};

typedef unsigned char BYTE;




std::string cleanString(const std::string& input);
std::string extractImageSection(const std::string& content);
std::string extractImageData(const std::string& imageSection);
std::vector<BYTE> base64_decode(std::string const& encoded_string);
void saveImageToPNG(const std::vector<unsigned char>& imageData, unsigned int width, unsigned int height, const std::string& filename);
Outil selectTool(double rayon, const std::string& classe) ;
int findToolPosition(const Outil& tool);
std::string getFilenameFromPath(const std::string& path);
std::string formatNumber(double value, int digits);
std::string getThickness(const std::string& filePath);
std::vector<PathCoordinates> extractPathAndCoordinates(const std::string& content);
std::string extractSym200(const std::string& symFileContent);
void printPathCoordinates(std::vector<PathCoordinates>& pathCoordinatesList);
std::vector<SymValue> parseValueSection(const std::string& valueSection);
std::vector<SymValue> extractSymData(const std::string& symFilePath);
std::vector<SymValueGroup> sortAndGroupByType(const std::vector<SymValue>& inputValues, const std::vector<int>& typeOrder);
bool sortByRayonAndDistance(const SymValue& a, const SymValue& b);
void printStructure(const std::vector<SymValue>& values);
SymValue transformation_repere_rad(const SymValue& point, const Coordinates& repere);
std::string generatePercageCommand(const PercageParams& params);
std::string generateFraisurageCommand(const FraisurageParams& params);
std::string generateTaraudageCommand(const TaraudageParams& params);
std::string generateLamageCommand(const LamageParams& params);
void generateCommands(const std::vector<SymValueGroup>& final_values, const std::string& epaisseur_tole);


#endif