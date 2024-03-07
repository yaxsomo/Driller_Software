cmake_minimum_required(VERSION 3.10)

project(IsiConnect-cpp-tests)

set(open62541pp "./open62541pp/build/open62541ppConfig.cmake")

# Find the open62541pp package
find_package(open62541pp CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

# Find Python
find_package(Python3 COMPONENTS Development)
include_directories(${Python3_INCLUDE_DIRS})

add_executable(IsiConnect-cpp-test main.cpp client
    client.cpp
    driller_frames.cpp)
set_property(TARGET IsiConnect-cpp-test PROPERTY CXX_STANDARD 17)
add_compile_options(-Wno-attributes -Wno-ignored-attributes)
set(CMAKE_CXX_FLAGS "-DBOOST_LOG_DYN_LINK")
target_include_directories(IsiConnect-cpp-test PUBLIC
                          "../include"
                          "../include/DAO"
                          "../libs/cpp-dotenv/include"
                          "../libs/msgpack")

# Link the open62541pp library
target_link_libraries(client PRIVATE open62541pp::open62541pp ${Python3_LIBRARIES})

target_link_libraries(IsiConnect-cpp-test PUBLIC IsiConnect-cpp nats pthread boost_thread boost_log boost_chrono cpp_dotenv msgpack11 ssl crypto dl)
# Install the .env file in the build directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.env)
    add_custom_command(TARGET IsiConnect-cpp-test POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CMAKE_CURRENT_SOURCE_DIR}/.env
                        $<TARGET_FILE_DIR:IsiConnect-cpp-test>
                    )
endif()

