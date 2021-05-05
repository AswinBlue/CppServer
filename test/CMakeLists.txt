cmake_minimum_required(VERSION 3.10)
project(testClient LANGUAGES CXX VERSION 0.0.1)
MESSAGE(project name : ${CMAKE_PROJECT_NAME})

SET ( SPDLOG_PATH ${CMAKE_SOURCE_DIR}/../lib/spdlog)
SET ( RAPIDJSON_PATH ${CMAKE_SOURCE_DIR}/../lib/rapidjson)
SET ( SERVER_CLIENT_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/../SC_framework)
SET ( MAIN_SRC_PATH ${CMAKE_SOURCE_DIR}/../src)
MESSAGE(SPDLOG_PATH : ${SPDLOG_PATH})
MESSAGE(RAPIDJSON_PATH: ${RAPIDJSON_PATH})
MESSAGE(\n)
#SET ( MYSQL_CONNECTOR_CPP_PATH ${CMAKE_SOURCE_DIR}/lib/mysql-connector-cpp)
#SET ( MYSQL_CONNECTOR_CPP_PATH /usr/local/mysql/connector-c++-8.0)

#set c++ library setting
SET (CMAKE_CXX_STANDARD 17)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
#SET (CMAKE_CXX_FLAGS "-fpermissive")

SET( SRC_FILES testClient.cpp)
add_executable(${CMAKE_PROJECT_NAME}.out ${SRC_FILES})

find_package(Boost 1.75.0 REQUIRED COMPONENTS filesystem thread)

# add header directories : same as '-I' option
target_include_directories(${CMAKE_PROJECT_NAME}.out
    # for /usr/local/include
    PRIVATE include
    # for spdlog
    PUBLIC ${SPDLOG_PATH}/include
    # for rapidjson
    ${RAPIDJSON_PATH}/include
    # for mysql-connector-cpp
    PUBLIC /usr/local/mysql/connector-c++-/include
    # for boost
    ${Boost_INCLUDE_DIRS}
    # for my custom framework
    ${SERVER_CLIENT_FRAMEWORK_PATH}
)

target_link_libraries( ${CMAKE_PROJECT_NAME}.out  LINK_PUBLIC ${Boost_LIBRARIES} )
target_include_directories(${CMAKE_PROJECT_NAME}.out
    # for /usr/local/include
    PRIVATE include
    # for spdlog
    PUBLIC ${SPDLOG_PATH}/include
    # for rapidjson
    ${RAPIDJSON_PATH}/include
    # for mysql-connector-cpp
    PUBLIC /usr/local/mysql/connector-c++-/include
    # for boost
    ${Boost_INCLUDE_DIRS}
    # for my custom framework
    ${SERVER_CLIENT_FRAMEWORK_PATH}
    # for headers in main source directory(src)
    ${MAIN_SRC_PATH}
)



