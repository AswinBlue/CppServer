cmake_minimum_required(VERSION 3.10)
project(RPG_server LANGUAGES CXX VERSION 0.0.1)
MESSAGE(project name : ${CMAKE_PROJECT_NAME})

SET ( SPDLOG_PATH ${CMAKE_SOURCE_DIR}/lib/spdlog)
SET ( RAPIDJSON_PATH ${CMAKE_SOURCE_DIR}/lib/rapidjson)
SET ( SERVER_CLIENT_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/SC_framework)
MESSAGE(SPDLOG_PATH : ${SPDLOG_PATH})
MESSAGE(RAPIDJSON_PATH: ${RAPIDJSON_PATH})
MESSAGE(\n)
#SET ( MYSQL_CONNECTOR_CPP_PATH ${CMAKE_SOURCE_DIR}/lib/mysql-connector-cpp)
#SET ( MYSQL_CONNECTOR_CPP_PATH /usr/local/mysql/connector-c++-8.0)

#set c++ library setting
SET (CMAKE_CXX_STANDARD 17)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
#SET (CMAKE_CXX_FLAGS "-fpermissive")

add_subdirectory(src)
#add_subdirectory(lib)

add_executable(${CMAKE_PROJECT_NAME}.out main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}.out
    PRIVATE main_source_lib
)

target_include_directories(${CMAKE_PROJECT_NAME}.out
    PRIVATE src
)
