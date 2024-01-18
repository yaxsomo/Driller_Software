#!/bin/bash

#install dependencies related to C++
sudo apt install build-essentials && sudo apt install make && sudo apt install cmake

# Add open62541pp library
git clone --recursive https://github.com/open62541pp/open62541pp.git
cd open62541pp/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUAPP_BUILD_EXAMPLES=ON -DUAPP_BUILD_TESTS=ON ..
cmake --build .
cmake --install .

# Remove CMakeCache.txt file and CMakeFiles folder
cd ..
rm -f CMakeCache.txt && rm -rf CMakeFiles


# Build the project
cmake .
make

# Execution privilege on client
sudo chmod +x client 
