cmake_minimum_required(VERSION 3.5)
project(Sphere-Linked)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")

add_executable(sphere-linked-server
        server/main.cpp
        server/config.hpp
        server/config.cpp
        server/connection.cpp
        server/connection.hpp
        server/message.cpp
        server/message.hpp
        server/shared.cpp
        server/shared.hpp
        server/opt.cpp
        server/opt.hpp
        server/socket_management.cpp
        server/socket_management.hpp
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

if (UNIX AND NOT APPLE)
    find_library(UUID_LIB uuid REQUIRED)
    target_link_libraries(sphere-linked-server PRIVATE ${UUID_LIB})
endif()