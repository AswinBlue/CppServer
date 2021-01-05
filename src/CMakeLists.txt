SET( SRC_FILES Logger.cpp Map.cpp  MySQL.cpp  Server.cpp)
SET ( CMAKE_BUILD_TYPE Debug )
SET ( CMAKE_VERBOSE_MAKEFILE true )
add_library(main_source_lib ${SRC_FILES})


target_link_libraries(main_source_lib
    # for spdlog
    PUBLIC spdlog::spdlog
    # for mysql-connector-cpp
)

target_include_directories(main_source_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    # for spdlog
#    PUBLIC ${CMAKE_SOURCE_DIR}/lib/spdlog/include
    PUBLIC ${SPDLOG_PATH}/include
    # for rapidjson
#    PUBLIC ${CMAKE_SOURCE_DIR}/lib/rapidjson/include
    PUBLIC ${RAPIDJSON_PATH}/include
    # for mysql-connector-cpp
    PUBLIC ${MYSQL_CONNECTOR_CPP_PATH}/include
)

