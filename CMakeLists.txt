cmake_minimum_required(VERSION 3.10)
project(vika-net)

if (WIN32)
    add_compile_definitions(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0602) # windows 8 and later
endif ()

file(GLOB SOURCE "src/*.cpp")
file(GLOB HEADER "include/*.hpp")

set(CMAKE_CXX_STANDARD 17)
find_package(spdlog REQUIRED)

add_library(vika-net STATIC ${SOURCE})

if (WIN32)
    target_link_libraries(vika-net PRIVATE spdlog::spdlog wsock32 ws2_32)
else()
    target_link_libraries(vika-net PRIVATE spdlog::spdlog)
endif()

add_executable(test-udp-server test/TestUDPServer.cpp)
target_link_libraries(test-udp-server PRIVATE vika-net)

add_executable(test-udp-client test/TestUDPClient.cpp)
target_link_libraries(test-udp-client PRIVATE vika-net)

add_executable(test-tcp-client test/TestTCPClient.cpp)
target_link_libraries(test-tcp-client PRIVATE vika-net)