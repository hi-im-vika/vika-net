cmake_minimum_required(VERSION 3.10)
project(vika-udp)

set(CMAKE_CXX_STANDARD 17)

add_library(${CMAKE_PROJECT_NAME} STATIC library.cpp)
