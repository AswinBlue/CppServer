SET( SRC_FILES Logger.cpp Map.cpp  MySQL.cpp  Server.cpp)
SET ( CMAKE_BUILD_TYPE Debug )
SET ( CMAKE_VERBOSE_MAKEFILE true )
add_library(main_source_lib ${SRC_FILES})

# boost
find_package(Boost COMPONENTS filesystem thread REQUIRED)

# add header directories : same as '-I' option
target_include_directories(main_source_lib
    # for spdlog
    PUBLIC ${SPDLOG_PATH}/include
    # for rapidjson
     ${RAPIDJSON_PATH}/include
    # for mysql-connector-cpp
    PUBLIC /usr/local/mysql/connector-c++-/include
    # for boost
    ${Boost_INCLUDE_DIRS}
)

#link_directories(${MYSQL_CONNECTOR_CPP_PATH}/include)

# add library paths: same as '-L' option
# for mysql-connector-cpp
target_link_directories(main_source_lib
    PUBLIC
    /usr/local/mysql/connector-c++-/lib64/debug
)
#find_library(mysqlcppconn8 /usr/local/mysql/connector-c++-/lib64/debug)

# add library file: same as '-l' option
target_link_libraries(main_source_lib
    PRIVATE
    # for spdlog
#    PUBLIC spdlog::spdlog
    # for boost
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
# for mysql
    mysqlcppconn8
)

