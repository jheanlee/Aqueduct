//
// Created by Jhean Lee on 2025/1/28.
//
#include <iostream>
#include <sstream>

#include "console.hpp"
#include "shared.hpp"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define YELLOW      "\033[33m"
#define FAINT_GRAY  "\033[2;90m"

void console(Level level, Code code, const char *detail, const std::string &function) {
  std::stringstream buffer;

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
    case INSTRUCTION:
      break;
  }
  buffer << RESET;

  //  code
  switch (code) {
    case SOCK_CREATE_FAILED:
      buffer << "Failed to create socket ";
      break;
    case SOCK_CONNECT_FAILED:
      buffer << "Failed to connect ";
      break;
    case SOCK_POLL_ERR:
      buffer << "An error has been returned by poll(), errno: ";
      break;
    case SSL_CONNECT_FAILED:
      buffer << "Failed to establish SSL connection with host ";
      break;
    case SSL_CREATE_CONTEXT_FAILED:
      buffer << "Failed to create SSL context ";
      break;
    case OPTION_UNKNOWN:
      buffer << "Unknown option passed to program. Please use the --help option to see usage ";
      break;
    case OPTION_SERVICE_PORT_NOT_SET:
      buffer << "Port of the service you want to stream is not set.  Please specify the port using the --service-port option ";
      break;
    case OPTION_TOKEN_NOT_SET:
      buffer << "Access token for connecting to host is not set. Please specify the token using the --token option ";
      break;
    case RESOLVE_HOST_FAILED:
      buffer << "Failed to resolve host address ";
      break;
    case PORT_INVALID_CHARACTER:
      buffer << "Invalid value passed as port number ";
      break;
    case PORT_INVALID_RANGE:
      buffer << "Invalid port range passed. Port numbers must be within the range of 1-65565 ";
      break;
    case INFO_HOST:
      buffer << "Host: ";
      break;
    case INFO_SERVICE:
      buffer << "Service: ";
      break;
    case MESSAGE_SEND_FAILED:
      buffer << "Failed to send message ";
      break;
    case MESSAGE_RECV_FAILED:
      buffer << "Failed to receive message ";
      break;
    case MESSAGE_LOAD_FAILED:
      buffer << "Failed to load message ";
      break;
    case MESSAGE_DUMP_FAILED:
      buffer << "Failed to dump message ";
      break;
    case BUFFER_SEND_ERROR_TO_SERVICE:
      buffer << "Failed to send buffer to service: ";
      break;
    case BUFFER_SEND_ERROR_TO_HOST:
      buffer << "Failed to send buffer to host: ";
      break;
    case CONNECTED_TO_HOST:
      buffer << "Connected to host: ";
      break;
    case CONNECTED_TO_SERVICE:
      buffer << "Connected to service: ";
      break;
    case CONNECTED_FOR_ID:
      buffer << "Connected to host for redirect id: ";
      break;
    case CONNECTION_CLOSED:
      buffer << "Connection has been closed: ";
      break;
    case CONNECTION_CLOSED_BY_SERVICE:
      buffer << "Connection has been closed by service: ";
      break;
    case CONNECTION_CLOSED_BY_HOST:
      buffer << "Connection has been closed by host: ";
      break;
    case STREAM_PORT_OPENED:
      buffer << "Service is now available at: ";
      break;
    case NO_PORTS_AVAILABLE:
      buffer << "Server has no ports available. Please try again later ";
    case PROXYING_STARTED:
      buffer << "Proxying started: ";
      break;
    case PROXYING_ENDED:
      buffer << "Proxying ended: ";
      break;
    case AUTHENTICATION_REQUEST_SENT:
      buffer << "Authentication request sent ";
      break;
    case AUTHENTICATION_FAILED:
      buffer << "Authentication failed. Now ending process ";
      break;
    case AUTHENTICATION_SUCCESS:
      buffer << "Authentication success ";
      break;
    case SHA256_FAILED:
      buffer << "Failed to hash token ";
      break;
    case SERVER_DATABASE_ERROR:
      buffer << "Server has returned an error ";
      break;
    case HOST_RESPONSE_TIMEOUT:
      buffer << "Host is not resposive. Timed out ";
      break;
    case ENTER_TOKEN_INSTRUCTION:
      buffer << "Please enter your token: ";
      break;
    case INVALID_TOKEN:
      buffer << "Invalid token format ";
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

  std::cout << buffer.str();
}