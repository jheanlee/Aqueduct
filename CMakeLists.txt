cmake_minimum_required(VERSION 3.5)
project(Sphere-Linked)
set(CMAKE_CXX_STANDARD 17)

add_executable(sphere-linked-server
        server/main.cpp
        server/tunnel/config.hpp
        server/tunnel/config.cpp
        server/tunnel/connection.cpp
        server/tunnel/connection.hpp
        server/tunnel/message.cpp
        server/tunnel/message.hpp
        server/tunnel/shared.cpp
        server/tunnel/shared.hpp
        server/tunnel/opt.cpp
        server/tunnel/opt.hpp
        server/tunnel/socket_management.cpp
        server/tunnel/socket_management.hpp
        server/database/auth.cpp
        server/database/auth.hpp
        server/tunnel/input.cpp
        server/tunnel/input.hpp
)
add_executable(sphere-linked-client
        client/main.cpp
        client/message.cpp
        client/message.hpp
        client/config.hpp
        client/connection.cpp
        client/connection.hpp
        client/shared.cpp
        client/shared.hpp
        client/opt.cpp
        client/opt.hpp
        client/socket_management.cpp
        client/socket_management.hpp
)

find_package(Threads)
target_link_libraries(sphere-linked-server PRIVATE Threads::Threads)
target_link_libraries(sphere-linked-client PRIVATE Threads::Threads)

target_link_libraries(sphere-linked-server PRIVATE ${CMAKE_DL_LIBS})
target_link_libraries(sphere-linked-client PRIVATE ${CMAKE_DL_LIBS})

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
target_link_libraries(sphere-linked-server PRIVATE OpenSSL::SSL)
target_link_libraries(sphere-linked-client PRIVATE OpenSSL::SSL)

find_package(SQLite3 REQUIRED)
target_link_libraries(sphere-linked-server PRIVATE SQLite::SQLite3)

if (UNIX AND NOT APPLE)
    find_library(UUID_LIB uuid REQUIRED)
    target_link_libraries(sphere-linked-server PRIVATE ${UUID_LIB})
endif()