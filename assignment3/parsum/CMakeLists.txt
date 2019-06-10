cmake_minimum_required(VERSION 3.10)
project(parsum)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(parsum parsum.cpp)