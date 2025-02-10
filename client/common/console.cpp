//
// Created by Jhean Lee on 2025/1/28.
//
#include <iostream>

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
    case INSTRUCTION:
      break;
  }
  output += RESET;

  //  code
  switch (code) {
    case SOCK_CREATE_FAILED:
      output += "Failed to create socket ";
      break;
    case SOCK_CONNECT_FAILED:
      output += "Failed to connect ";
      break;
    case SOCK_POLL_ERR:
      output += "An error has been returned by poll(), errno: ";
      break;
    case SSL_CONNECT_FAILED:
      output += "Failed to establish SSL connection with host ";
      break;
    case SSL_CREATE_CONTEXT_FAILED:
      output += "Failed to create SSL context ";
      break;
    case OPTION_UNKNOWN:
      output += "Unknown option passed to program. Please use the --help option to see usage ";
      break;
    case OPTION_SERVICE_PORT_NOT_SET:
      output += "Port of the service you want to stream is not set.  Please specify the port using the --service-port option ";
      break;
    case OPTION_TOKEN_NOT_SET:
      output += "Access token for connecting to host is not set. Please specify the token using the --token option ";
      break;
    case RESOLVE_HOST_FAILED:
      output += "Failed to resolve host address ";
      break;
    case PORT_INVALID_CHARACTER:
      output += "Invalid value passed as port number ";
      break;
    case PORT_INVALID_RANGE:
      output += "Invalid port range passed. Port numbers must be within the range of 1-65565 ";
      break;
    case INFO_HOST:
      output += "Host: ";
      break;
    case INFO_SERVICE:
      output += "Service: ";
      break;
    case MESSAGE_SEND_FAILED:
      output += "Failed to send message ";
      break;
    case MESSAGE_RECV_FAILED:
      output += "Failed to receive message ";
      break;
    case MESSAGE_LOAD_FAILED:
      output += "Failed to load message ";
      break;
    case MESSAGE_DUMP_FAILED:
      output += "Failed to dump message ";
      break;
    case BUFFER_SEND_ERROR_TO_SERVICE:
      output += "Failed to send buffer to service: ";
      break;
    case BUFFER_SEND_ERROR_TO_HOST:
      output += "Failed to send buffer to host: ";
      break;
    case CONNECTED_TO_HOST:
      output += "Connected to host: ";
      break;
    case CONNECTED_TO_SERVICE:
      output += "Connected to service: ";
      break;
    case CONNECTED_FOR_ID:
      output += "Connected to host for redirect id: ";
      break;
    case CONNECTION_CLOSED:
      output += "Connection has been closed: ";
      break;
    case CONNECTION_CLOSED_BY_SERVICE:
      output += "Connection has been closed by service: ";
      break;
    case CONNECTION_CLOSED_BY_HOST:
      output += "Connection has been closed by host: ";
      break;
    case STREAM_PORT_OPENED:
      output += "Service is now available at: ";
      break;
    case PROXYING_STARTED:
      output += "Proxying started: ";
      break;
    case PROXYING_ENDED:
      output += "Proxying ended: ";
      break;
    case AUTHENTICATION_REQUEST_SENT:
      output += "Authentication request sent ";
      break;
    case AUTHENTICATION_FAILED:
      output += "Authentication failed. Now ending process ";
      break;
    case AUTHENTICATION_SUCCESS:
      output += "Authentication success ";
      break;
    case SHA256_FAILED:
      output += "Failed to hash token ";
      break;
    case SERVER_DATABASE_ERROR:
      output += "Server has returned an error ";
      break;
    case HOST_RESPONSE_TIMEOUT:
      output += "Host is not resposive. Timed out ";
      break;
    case ENTER_TOKEN_INSTRUCTION:
      output += "Please enter your token: ";
      break;
    case INVALID_TOKEN:
      output += "Invalid token format ";
      break;
  }

  if (detail != nullptr) {
    output += detail;
    output += ' ';
  }

  if (verbose) {
    output += FAINT_GRAY;
    output += '(';
    output += function;
    output += ')';
    output += RESET;
  }
  output += '\n';

  std::cout << output;
}