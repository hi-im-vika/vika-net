cmake_minimum_required(VERSION 3.10)
project(vika-udp)

file(GLOB SOURCE "src/*.cpp")
file(GLOB HEADER "include/*.h")

set(CMAKE_CXX_STANDARD 17)
find_package(spdlog REQUIRED)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCE})
target_link_libraries(${CMAKE_PROJECT_NAME} spdlog::spdlog)