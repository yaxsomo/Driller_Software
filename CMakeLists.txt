cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(open62541pp "./open62541pp/build/open62541ppConfig.cmake")

# Find the open62541pp package
find_package(open62541pp CONFIG REQUIRED)

# Add your source files
add_executable(client
    client.cpp
    driller_frames.cpp
)

# Link the open62541pp library
target_link_libraries(client PRIVATE open62541pp::open62541pp)
