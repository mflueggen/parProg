cmake_minimum_required(VERSION 3.10)
project(heatmap)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DBENCHMARK -DCL_TARGET_OPENCL_VERSION=120)

find_package(OpenCL REQUIRED)
add_executable(heatmap main.cpp)
target_link_libraries(heatmap OpenCL::OpenCL)
