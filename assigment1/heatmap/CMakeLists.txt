cmake_minimum_required(VERSION 3.13)
project(heatmap)

SET(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(heatmap main.cpp)

