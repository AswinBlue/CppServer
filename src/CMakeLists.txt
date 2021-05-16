SET( SRC_FILES MySQL.cpp)
SET ( CMAKE_BUILD_TYPE Debug )
SET ( CMAKE_VERBOSE_MAKEFILE true )
add_library(main_source_lib ${SRC_FILES})

# boost
# don't need to add asio here
find_package(Boost 1.75.0 REQUIRED COMPONENTS filesystem thread)

# add header directories : same as '-I' option
target_include_directories(main_source_lib
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

#link_directories(${MYSQL_CONNECTOR_CPP_PATH}/include)

# add library paths: same as '-L' option
target_link_directories(main_source_lib
    # for mysql-connector-cpp
    PUBLIC
    /usr/local/mysql/connector-c++-/lib64/debug
)
#find_library(mysqlcppconn8 /usr/local/mysql/connector-c++-/lib64/debug)

# add library file: same as '-l' option
# if omit this, link error occur
target_link_libraries(main_source_lib
    PRIVATE
    # for spdlog
#    PUBLIC spdlog::spdlog
    # for boost
#    ${Boost_FILESYSTEM_LIBRARY}
#    ${Boost_THREAD_LIBRARY}
#    ${Boost_ASIO_LIBRARY}
     ${Boost_LIBRARIES}
# for mysql
    mysqlcppconn8
)

