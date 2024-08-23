cmake_minimum_required(VERSION 3.0)
project(Tunnel)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}../../bin)

add_executable(server server_main.cpp server/server_util.cpp server/server_util.hpp common/message.hpp common/message.cpp
        server/server_config.hpp server/server_config.cpp
        common/random_key.cpp
        common/random_key.hpp)
add_executable(client client_main.cpp client/client_util.hpp client/client_util.cpp common/message.hpp common/message.cpp
        client/client_config.cpp
        client/client_config.hpp
)