cmake_minimum_required(VERSION 3.10)
project(hoi)

set(CMAKE_CXX_STANDARD 17)

add_executable(hoi main.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(hoi PUBLIC OpenMP::OpenMP_CXX)
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_CRYPTO_LIBRARY)
    target_link_libraries(hoi PUBLIC OpenSSL::Crypto)
endif()
