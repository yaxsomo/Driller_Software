#!/bin/bash

sudo apt-get update && sudo apt-get upgrade -y 
sudo apt update && sudo apt upgrade -y

#install dependencies related to C++
sudo apt install build-essential make cmake -y
sudo apt-get install openssl libssl-dev -y
sudo apt-get install libboost-all-dev -y

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

cd libs/
git clone git@github.com:nats-io/nats.c.git 
cd nats.c/ && mkdir build && cd build/
cmake .. -DNATS_BUILD_STREAMING=OFF && make && sudo make install
cd ../../..

# Build the project
cmake .
make

#Install Python 3
sudo apt install python3 python3.10-dev -y
#Install Pip and Pillow library
sudo apt install python3-pip -y && pip3 install Pillow

# Execution privilege on client
sudo chmod +x ./client 
