cmake_minimum_required(VERSION 3.10)
project(Asio_Client LANGUAGES CXX VERSION 0.0.1)

SET ( SRC_FILES AsioClient.cpp )
SET ( CMAKE_BUILD_TYPE Debug )
add_executable( client.out ${SRC_FILES} )

find_package(Boost 1.75.0 REQUIRED COMPONENTS filesystem thread)

target_include_directories( client.out
    PUBLIC include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries( client.out
    PRIVATE
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_ASIO_LIBRARY}
)

