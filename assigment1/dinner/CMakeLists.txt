cmake_minimum_required(VERSION 3.10)
project(dinner)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -pthread)

add_executable(dinner main.cpp)