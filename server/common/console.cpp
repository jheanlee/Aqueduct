//
// Created by Jhean Lee on 2025/1/24.
//
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <mutex>

#include "console.hpp"
#include "shared.hpp"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define YELLOW      "\033[33m"
#define FAINT_GRAY  "\033[2;90m"
#define CYAN        "\033[36m"

void console(Level level, Code code, const char *detail, const std::string &function) {
  std::ostringstream buffer;

  //  timestamp
  char strtime[32];
  time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::strftime(strtime, 32, "(%Y-%m-%d %H:%M:%S) ", std::gmtime(&time));
  buffer << strtime;

  //  level
  switch (level) {
    case ERROR:
      buffer << RED;
      buffer << "[Error] ";
      break;
    case WARNING:
      buffer << YELLOW;
      buffer << "[Warning] ";
      break;
    case INFO:
      buffer << "[Info] ";
      break;
    case DEBUG:
      if (!verbose) {
        return;
      }
      buffer << CYAN;
      buffer << "[DEBUG] ";
      break;
  }
  buffer << RESET;

  //  code
  switch (code) {
    case SOCK_CREATE_FAILED:
      buffer << "Failed to create socket ";
      break;
    case SOCK_BIND_FAILED:
      buffer << "Failed to bind to socket ";
      break;
    case SOCK_LISTEN_FAILED:
      buffer << "Failed to listen on socket ";
      break;
    case SOCK_ACCEPT_FAILED:
      buffer << "Failed to accept on socket ";
      break;
    case SOCK_SETSOCKOPT_FAILED:
      buffer << "Failed to set socket options ";
      break;
    case SOCK_POLL_ERR:
      buffer << "An error has been returned by poll(), errno: ";
      break;
    case SSL_CREATE_CONTEXT_FAILED:
      buffer << "Failed to create SSL context ";
      break;
    case SSL_ACCEPT_FAILED:
      buffer << "Failed to accept SSL connection ";
      break;
    case SSL_LOAD_CERT_KEY_FAILED:
      buffer << "Failed to load SSL certificate or map_key ";
      break;
    case SQLITE_OPEN_FAILED:
      buffer << "Failed to open SQLite database\n";
      break;
    case SQLITE_CREATE_TABLE_FAILED:
      buffer << "Failed to create SQLite table\n";
      break;
    case SQLITE_PREPARE_FAILED:
      buffer << "Failed to prepare SQL statement\n";
      break;
    case SQLITE_BIND_PARAMETER_FAILED:
      buffer << "Failed to bind SQL parameter\n";
      break;
    case SQLITE_STEP_FAILED:
      buffer << "Failed to execute SQL statement\n";
      break;
    case SQLITE_RETRIEVE_FAILED:
      buffer << "Failed to retrieve data from database\n";
      break;
    case SQLITE_CLOSING:
      buffer << "Closing SQLite database ";
      break;
    case SQLITE_CLOSE_SUCCESS:
      buffer << "Successfully closed SQLite database ";
      break;
    case SQLITE_CLOSE_FAILED:
      buffer << "Failed to close SQLite database\n";
      break;
    case INVALID_DB:
      buffer << "Invalid database pointer ";
      break;
    case GENERATED_TOKEN:
      buffer << "A new token has been generated for ";
      break;
    case REMOVED_TOKEN:
      buffer << "Token removed successfully ";
      break;
    case OPTION_UNKNOWN:
      buffer << "Unknown option passed to program. Please use the --help option to see usage ";
      break;
    case OPTION_KEY_NOT_SET:
      buffer << "Key for TLS connection is not set. Please specify the path to the private map_key using the --tls-map_key option ";
      break;
    case OPTION_CERT_NOT_SET:
      buffer << "Certificate for TLS connection is not set. Please specify the path to the certificate using the --tls-cert option ";
      break;
    case PORT_INVALID_CHARACTER:
      buffer << "Invalid value passed as port number ";
      break;
    case PORT_INVALID_RANGE:
      buffer << "Invalid port range passed. Port numbers must be within the range of 1-65565 ";
      break;
    case PORT_WELL_KNOWN:
      buffer << "Well-known ports (range 1-1023) passed. May require escalated privilages to bind ";
      break;
    case PORT_INVALID_LIMIT:
      buffer << "Invalid port limit ";
      break;
    case PORT_MAY_EXCEED:
      buffer << "Port numbers may exceed 65535 ";
      break;
    case INFO_KEY_PATH:
      buffer << "TLS private map_key: ";
      break;
    case INFO_CERT_PATH:
      buffer << "TLS certificate: ";
      break;
    case INFO_DB_PATH:
      buffer << "Database file: ";
      break;
    case INFO_HOST:
      buffer << "Streaming host: ";
      break;
    case MESSAGE_SEND_FAILED:
      buffer << "Failed to send message ";
      break;
    case MESSAGE_LOAD_FAILED:
      buffer << "Failed to load message ";
      break;
    case MESSAGE_DUMP_FAILED:
      buffer << "Failed to dump message ";
      break;
    case BUFFER_SEND_ERROR_TO_CLIENT:
      buffer << "Failed to send buffer to client: ";
      break;
    case BUFFER_SEND_ERROR_TO_EXTERNAL_USER:
      buffer << "Failed to send buffer to external user: ";
      break;
    case CONNECTION_LISTEN_STARTED:
      buffer << "Listening for connection ";
      break;
    case CLIENT_CONNECTION_ACCEPTED:
      buffer << "Accepted connection from client: ";
      break;
    case EXTERNAL_CONNECTION_ACCEPTED:
      buffer << "Accepted external connection: ";
      break;
    case CONNECTION_CLOSED:
      buffer << "Connection has been closed: ";
      break;
    case CONNECTION_CLOSED_BY_CLIENT:
      buffer << "Proxy connection has been closed by client: ";
      break;
    case CONNECTION_CLOSED_BY_EXTERNAL_USER:
      buffer << "Proxy connection has been closed by external user ";
      break;
    case HEARTBEAT_TIMEOUT:
      buffer << "Client heartbeat timed out: ";
      break;
    case AUTHENTICATION_FAILED:
      buffer << "Client authentication failed: ";
      break;
    case AUTHENTICATION_SUCCESS:
      buffer << "Client authentication success: ";
      break;
    case PROXY_PORT_NEW:
      buffer << "Opened new proxy port: ";
      break;
    case PROXYING_STARTED:
      buffer << "Proxying started: ";
      break;
    case PROXYING_ENDED:
      buffer << "Proxying ended: ";
      break;
    case NO_PORT_AVAILABLE:
      buffer << "No available ports ";
      break;
    case SIGNAL:
      buffer << "Closing with signal ";
      break;
    case DEBUG_MSG:
      break;
  }

  if (detail != nullptr) {
    buffer << detail;
    buffer << ' ';
  }

  if (verbose) {
    buffer << FAINT_GRAY;
    buffer << '(';
    buffer << function;
    buffer << ')';
    buffer << RESET;
  }
  buffer << '\n';

  std::lock_guard<std::mutex> cout_lock(shared_resources::cout_mutex);
  std::cout << buffer.str();
}
