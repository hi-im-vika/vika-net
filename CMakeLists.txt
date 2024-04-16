cmake_minimum_required(VERSION 3.10)
project(vika-udp)

file(GLOB SOURCE "src/*.cpp")
file(GLOB HEADER "include/*.h")

set(CMAKE_CXX_STANDARD 17)
find_package(spdlog REQUIRED)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCE}
)
target_link_libraries(${CMAKE_PROJECT_NAME} spdlog::spdlog)

add_executable(test-server test/TestServer.cpp)
target_link_libraries(test-server ${CMAKE_PROJECT_NAME})

add_executable(test-client test/TestClient.cpp)
target_link_libraries(test-client ${CMAKE_PROJECT_NAME})