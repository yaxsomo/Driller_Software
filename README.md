# Driller_Software
This Repository is reserved for LUCAS's Smart Driller Robot Software developement


### Adding the open62541pp library
In order to use the open62541pp library, we have to build it first. Here's the commands to execute in order :
```bash
git clone --recursive https://github.com/open62541pp/open62541pp.git
cd open62541pp/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUAPP_BUILD_EXAMPLES=ON -DUAPP_BUILD_TESTS=ON ..
cmake --build .
cmake --install .        
```

### Building the project
Then we can go into the project root location and use the following commands to build the entire project :

```bash
cmake .
cd build
make
```

Now the project is built successfully!

### Executing the main program

In order to execute the program, we have to go into the build folder and launch the client file :

Usage 
```
./client [filename.drg]
```
Exemple with a known file name : 
```bash
cd build
 ./client P1_TITRE.drg 
```