cmake_minimum_required(VERSION 3.14)
project(mpiavg)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpic++)

set(SOURCE_FILES main.c)

add_executable(mpiavg main.cpp)