cmake_minimum_required(VERSION 3.5)
project(Aqueduct)
set(CMAKE_CXX_STANDARD 17)

#   server core
add_executable(aqueduct-server
        server/main.cpp
        server/core/tunnel/connection.cpp
        server/core/tunnel/connection.hpp
        server/core/tunnel/message.cpp
        server/core/tunnel/message.hpp
        server/core/common/shared.cpp
        server/core/common/shared.hpp
        server/core/common/opt.cpp
        server/core/common/opt.hpp
        server/core/tunnel/socket_management.cpp
        server/core/tunnel/socket_management.hpp
        server/core/database/database.cpp
        server/core/database/database.hpp
        server/core/common/console.cpp
        server/core/common/console.hpp
        server/core/common/signal_handler.cpp
        server/core/common/signal_handler.hpp
        server/core/database/authentication.cpp
        server/core/database/authentication.hpp
        server/core/database/client.cpp
        server/core/database/client.hpp
        server/core/common/input.cpp
        server/core/common/input.hpp
        server/core/children/api.cpp
        server/core/children/api.hpp
        server/core/children/token.cpp
        server/core/children/token.hpp
)

#   client core
add_executable(aqueduct-client
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

#   server/client dependencies
find_package(Threads)
target_link_libraries(aqueduct-server PRIVATE Threads::Threads)
target_link_libraries(aqueduct-client PRIVATE Threads::Threads)

target_link_libraries(aqueduct-server PRIVATE ${CMAKE_DL_LIBS})
target_link_libraries(aqueduct-client PRIVATE ${CMAKE_DL_LIBS})

set(OPENSSL_USE_STATIC_LIBS TRUE)   #   if static openssl library not found, comment out this line to use dynamic linking
find_package(OpenSSL REQUIRED)
target_link_libraries(aqueduct-server PRIVATE OpenSSL::SSL)
target_link_libraries(aqueduct-client PRIVATE OpenSSL::SSL)

find_package(SQLite3 REQUIRED)
target_link_libraries(aqueduct-server PRIVATE SQLite::SQLite3)

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(aqueduct-server PRIVATE CLI11::CLI11)
target_link_libraries(aqueduct-client PRIVATE CLI11::CLI11)

find_package(nlohmann_json REQUIRED)
target_link_libraries(aqueduct-server PRIVATE nlohmann_json::nlohmann_json)

if (UNIX AND NOT APPLE)
    find_library(UUID_LIB uuid REQUIRED)
    target_link_libraries(aqueduct-server PRIVATE ${UUID_LIB})
endif ()

#   cargo
if (DEFINED ENV{CARGO_BIN_PATH})
    set(CARGO_BIN_PATH $ENV{CARGO_BIN_PATH})
else()
    set(CARGO_BIN_PATH cargo)
endif()
message(STATUS "Cargo path: ${CARGO_BIN_PATH}")

#   npm
if (DEFINED ENV{NPM_BIN_PATH)
    set(NPM_BIN_PATH $ENV{NPM_BIN_PATH})
else()
    set(NPM_BIN_PATH npm)
endif()
message(STATUS "npm path: ${NPM_BIN_PATH}")

#   server api
add_custom_command(TARGET aqueduct-server POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/server/api
        COMMAND ${CARGO_BIN_PATH} build --release --target-dir ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/release/api ${CMAKE_BINARY_DIR}/aqueduct-server-api
)

#   server frontend
add_custom_command(TARGET aqueduct-server POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/server/frontend
        COMMAND ${NPM_BIN_PATH} run build -- --outDir ${CMAKE_BINARY_DIR}/frontend
)