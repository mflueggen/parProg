cmake_minimum_required(VERSION 3.14)
project(mpiavg)

set(CMAKE_CXX_STANDARD 17)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpic++)

set(SOURCE_FILES mpiavg.cpp)

add_executable(mpiavg mpiavg.cpp)