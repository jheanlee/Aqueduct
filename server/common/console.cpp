//
// Created by Jhean Lee on 2025/1/24.
//
#include <iostream>
#include <chrono>
#include <ctime>

#include "console.hpp"
#include "shared.hpp"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define YELLOW      "\033[33m"
#define FAINT_GRAY  "\033[2;90m"

void console(Level level, Code code, const char *detail, const std::string &function) {
  std::string output;

  //  timestamp
  char strtime[32];
  time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::strftime(strtime, 32, "(%Y-%m-%d %H:%M:%S) ", std::gmtime(&time));
  output += strtime;

  //  level
  switch (level) {
    case ERROR:
      output += RED;
      output += "[Error] ";
      break;
    case WARNING:
      output += YELLOW;
      output += "[Warning] ";
      break;
    case INFO:
      output += "[Info] ";
      break;
  }
  output += RESET;

  //  code
  switch (code) {
    case SOCK_CREATE_FAILED:
      output += "Failed to create socket ";
      break;
    case SOCK_BIND_FAILED:
      output += "Failed to bind to socket ";
      break;
    case SOCK_LISTEN_FAILED:
      output += "Failed to listen on socket ";
      break;
    case SOCK_ACCEPT_FAILED:
      output += "Failed to accept on socket ";
      break;
    case SOCK_SETSOCKOPT_FAILED:
      output += "Failed to set socket options ";
      break;
    case SOCK_SELECT_INVALID_FD:
      output += "Invalid file descriptor has been passed to select ";
      break;
    case SSL_CREATE_CONTEXT_FAILED:
      output += "Failed to create SSL context ";
      break;
    case SSL_ACCEPT_FAILED:
      output += "Failed to accept SSL connection ";
      break;
    case SSL_LOAD_CERT_KEY_FAILED:
      output += "Failed to load SSL certificate or key ";
      break;
    case SQLITE_OPEN_FAILED:
      output += "Failed to open SQLite database\n";
      break;
    case SQLITE_CREATE_TABLE_FAILED:
      output += "Failed to create SQLite table\n";
      break;
    case SQLITE_PREPARE_FAILED:
      output += "Failed to prepare SQL statement\n";
      break;
    case SQLITE_BIND_PARAMETER_FAILED:
      output += "Failed to bind SQL parameter\n";
      break;
    case SQLITE_RETRIEVE_FAILED:
      output += "Failed to retrieve data from database\n";
      break;
    case SQLITE_CLOSING:
      output += "Closing SQLite database ";
      break;
    case SQLITE_CLOSE_SUCCESS:
      output += "Successfully closed SQLite database ";
      break;
    case SQLITE_CLOSE_FAILED:
      output += "Failed to close SQLite database\n";
      break;
    case OPTION_UNKNOWN:
      output += "Unknown option passed to program. Please use the --help option to see usage ";
      break;
    case OPTION_KEY_NOT_SET:
      output += "Key for TLS connection is not set. Please specify the path to the private key using the --tls-key option ";
      break;
    case OPTION_CERT_NOT_SET:
      output += "Certificate for TLS connection is not set. Please specify the path to the certificate using the --tls-cert option ";
      break;
    case PORT_INVALID_CHARACTER:
      output += "Invalid port number passed as port number ";
      break;
    case PORT_INVALID_RANGE:
      output += "Invalid port range passed. Port numbers should be in the range of 1-65565 ";
      break;
    case PORT_WELL_KNOWN:
      output += "Well-known ports (range 1-1023) passed. May require escalated privilages to bind ";
      break;
    case PORT_INVALID_LIMIT:
      output += "Invalid port limit ";
      break;
    case PORT_MAY_EXCEED:
      output += "Port numbers may exceed 65535 ";
      break;
    case INFO_KEY_PATH:
      output += "TLS private key: ";
      break;
    case INFO_CERT_PATH:
      output += "TLS certificate: ";
      break;
    case INFO_DB_PATH:
      output += "Database file: ";
      break;
    case INFO_HOST:
      output += "Streaming host: ";
      break;
    case MESSAGE_SEND_FAILED:
      output += "Failed to send message ";
      break;
    case MESSAGE_LOAD_FAILED:
      output += "Failed to load message ";
      break;
    case MESSAGE_DUMP_FAILED:
      output += "Failed to dump message ";
      break;
    case BUFFER_SEND_ERROR_TO_CLIENT:
      output += "Failed to send buffer to client: ";
      break;
    case BUFFER_SEND_ERROR_TO_EXTERNAL_USER:
      output += "Failed to send buffer to external user: ";
      break;
    case CONNECTION_LISTEN_STARTED:
      output += "Listening for connection ";
      break;
    case CLIENT_CONNECTION_ACCEPTED:
      output += "Accepted connection from client: ";
      break;
    case EXTERNAL_CONNECTION_ACCEPTED:
      output += "Accepted external connection: ";
      break;
    case CONNECTION_CLOSED:
      output += "Connection has been closed: ";
      break;
    case CONNECTION_CLOSED_BY_CLIENT:
      output += "Connection has been closed by client: ";
      break;
    case CONNECTION_CLOSED_BY_EXTERNAL_USER:
      output += "Connection has been closed by external user ";
      break;
    case HEARTBEAT_TIMEOUT:
      output += "Client heartbeat timed out: ";
      break;
    case AUTHENTICATION_FAILED:
      output += "Client authentication failed: ";
      break;
    case AUTHENTICATION_SUCCESS:
      output += "Client authentication success: ";
      break;
    case PROXY_PORT_NEW:
      output += "Opened new proxy port: ";
      break;
    case PROXYING_STARTED:
      output += "Proxying started: ";
      break;
    case PROXYING_ENDED:
      output += "Proxying ended: ";
      break;
    case NO_PORT_AVAILABLE:
      output += "No available ports ";
      break;
    case SIGNAL:
      output += "Closing with signal ";
      break;
  }

  if (detail != nullptr) {
    output += detail;
    output += ' ';
  }

  if (verbose) {
    output += FAINT_GRAY;
    output += function;
    output += RESET;
  }
  output += '\n';

  std::cout << output;
}
