cmake_minimum_required(VERSION 3.5)
project(Sphere-Linked)
set(CMAKE_CXX_STANDARD 17)

add_executable(sphere-linked-server
        server/main.cpp
        server/tunnel/connection.cpp
        server/tunnel/connection.hpp
        server/tunnel/message.cpp
        server/tunnel/message.hpp
        server/common/shared.cpp
        server/common/shared.hpp
        server/common/opt.cpp
        server/common/opt.hpp
        server/tunnel/socket_management.cpp
        server/tunnel/socket_management.hpp
        server/database/database.cpp
        server/database/database.hpp
        server/tunnel/input.cpp
        server/tunnel/input.hpp
        server/common/console.cpp
        server/common/console.hpp
        server/common/signal_handler.cpp
        server/common/signal_handler.hpp
        server/database/authentication.cpp
        server/database/authentication.hpp
)
add_executable(sphere-linked-client
        client/main.cpp
        client/tunnel/message.cpp
        client/tunnel/message.hpp
        client/common/config.hpp
        client/tunnel/connection.cpp
        client/tunnel/connection.hpp
        client/common/shared.cpp
        client/common/shared.hpp
        client/common/opt.cpp
        client/common/opt.hpp
        client/tunnel/socket_management.cpp
        client/tunnel/socket_management.hpp
        client/common/console.cpp
        client/common/console.hpp
        client/common/signal_handler.cpp
        client/common/signal_handler.hpp
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

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(sphere-linked-server PRIVATE CLI11::CLI11)

if (UNIX AND NOT APPLE)
    find_library(UUID_LIB uuid REQUIRED)
    target_link_libraries(sphere-linked-server PRIVATE ${UUID_LIB})
endif()