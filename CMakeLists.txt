cmake_minimum_required(VERSION 3.0)
project(Tunnel)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}../../bin)

add_executable(server
        server/main.cpp
        server/config.hpp
        server/config.cpp
        server/connection.cpp
        server/connection.hpp
        server/message.cpp
        server/message.hpp)
add_executable(client
        client/main.cpp
        client/message.cpp
        client/message.hpp
        client/config.cpp
        client/config.hpp
        client/connection.cpp
        client/connection.hpp)