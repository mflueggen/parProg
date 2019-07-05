cmake_minimum_required(VERSION 2.6)

project(heatmap)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{METAL_ROOT}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # Put binaries into root of build tree

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -std=gnu11")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -std=c++17")

find_package(jq REQUIRED)
find_package(snap REQUIRED)
find_package(cxl REQUIRED)

include_directories($ENV{METAL_ROOT}/src)
link_directories($ENV{METAL_ROOT}/build/src/metal_pipeline)

SET (SOURCES
    heatmap.cpp
)

SET (LIBRARIES
    metal_pipeline
    ${JQ_LIBRARIES}
    ${SNAP_LIBRARY}
    ${CXL_LIBRARY}
)

add_executable(heatmap ${SOURCES})

target_link_libraries(heatmap ${LIBRARIES})
