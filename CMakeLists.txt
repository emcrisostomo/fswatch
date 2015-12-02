cmake_minimum_required(VERSION 3.3)
project(fswatch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-DHAVE_CONFIG_H)

add_subdirectory(libfswatch)
add_subdirectory(src)