cmake_minimum_required(VERSION 3.10)
project(vika-net)

file(GLOB SOURCE "src/*.cpp")
file(GLOB HEADER "include/*.hpp")

set(CMAKE_CXX_STANDARD 17)
find_package(spdlog REQUIRED)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCE})
target_link_libraries(${CMAKE_PROJECT_NAME} spdlog::spdlog)

add_executable(test-udp-server test/TestUDPServer.cpp)
target_link_libraries(test-udp-server ${CMAKE_PROJECT_NAME})

add_executable(test-udp-client test/TestUDPClient.cpp)
target_link_libraries(test-udp-client ${CMAKE_PROJECT_NAME})

add_executable(test-tcp-client test/TestTCPClient.cpp)
target_link_libraries(test-tcp-client ${CMAKE_PROJECT_NAME})