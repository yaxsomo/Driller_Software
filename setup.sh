#!/bin/bash

sudo apt-get update && sudo apt-get upgrade -y 
sudo apt update && sudo apt upgrade -y

#install dependencies related to C++
sudo apt install build-essential make cmake -y

# Add open62541pp library
git clone --recursive https://github.com/open62541pp/open62541pp.git
cd open62541pp/
mkdir build
cd build
sudo cmake -DCMAKE_BUILD_TYPE=Debug -DUAPP_BUILD_EXAMPLES=ON -DUAPP_BUILD_TESTS=ON ..
sudo cmake --build .
sudo cmake --install .

# Remove CMakeCache.txt file and CMakeFiles folder
cd ..
rm -f CMakeCache.txt && rm -rf CMakeFiles


# Build the project
cmake .
make

#Install Python 3
sudo apt install python3 python3.10-dev -y
#Install Pip and Pillow library
sudo apt install python3-pip -y && pip3 install Pillow

# Execution privilege on client
sudo chmod +x ./client 
