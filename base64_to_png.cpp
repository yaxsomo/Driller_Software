#include <iostream>
#include <fstream>
#include <Python.h>

//Compiling : 
// g++ -o base64_to_png base64_to_png.cpp -I /usr/include/python3.10 -lpython3.10

int main() {
    // Read base64 image from file
    std::ifstream file("image.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    std::string base64Image((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    // Initialize Python
    Py_Initialize();

    // Import required Python modules
    PyRun_SimpleString("import base64");
    PyRun_SimpleString("from PIL import Image");
    PyRun_SimpleString("from io import BytesIO");

    // Prepare Python code to decode base64 and save the image
    std::string pythonCode = "data = '''" + base64Image + "'''\n"
                             "im = Image.open(BytesIO(base64.b64decode(data)))\n"
                             "im.save('image.png', 'PNG')\n";

    // Execute Python code
    PyRun_SimpleString(pythonCode.c_str());

    // Finalize Python
    Py_Finalize();

    return 0;
}
