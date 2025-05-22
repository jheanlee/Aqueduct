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
#define CYAN        "\033[36m"

//  TODO system log

void console(Level level, Code code, const char *detail, const std::string &function) {
  if (level < verbose_level && level != INSTRUCTION) return;

  std::stringstream cout_buffer, msg_buffer;

  //  timestamp
  char strtime[32];
  time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::strftime(strtime, 32, "(%Y-%m-%d %H:%M:%S) ", std::gmtime(&time));
  cout_buffer << strtime;

  //  level
  switch (level) {
    case CRITICAL:
      cout_buffer << RED;
      cout_buffer << "[Critical] ";
      break;
    case ERROR:
      cout_buffer << RED;
      cout_buffer << "[Error] ";
      break;
    case WARNING:
      cout_buffer << YELLOW;
      cout_buffer << "[Warning] ";
      break;
    case INFO:
      cout_buffer << "[Info] ";
      break;
    case DEBUG:
      cout_buffer << "[Debug] ";
      break;
    case INSTRUCTION:
      break;
  }
  cout_buffer << RESET;

  //  code
  switch (code) {
    case SOCK_CREATE_FAILED:
      msg_buffer << "Failed to create socket";
      break;
    case SOCK_CONNECT_FAILED:
      msg_buffer << "Failed to connect";
      break;
    case SOCK_POLL_ERR:
      msg_buffer << "An error has been returned by poll(), errno:";
      break;
    case SSL_CONNECT_FAILED:
      msg_buffer << "Failed to establish SSL connection with host";
      break;
    case SSL_CREATE_CONTEXT_FAILED:
      msg_buffer << "Failed to create SSL context";
      break;
    case OPTION_UNKNOWN:
      msg_buffer << "Unknown option passed to program. Please use the --help option to see usage";
      break;
    case OPTION_SERVICE_PORT_NOT_SET:
      msg_buffer << "Port of the service you want to stream is not set.  Please specify the port using the --service-port option";
      break;
    case OPTION_TOKEN_NOT_SET:
      msg_buffer << "Access token for connecting to host is not set. Please specify the token using the --token option";
      break;
    case RESOLVE_HOST_FAILED:
      msg_buffer << "Failed to resolve host address";
      break;
    case PORT_INVALID_CHARACTER:
      msg_buffer << "Invalid value passed as port number";
      break;
    case PORT_INVALID_RANGE:
      msg_buffer << "Invalid port range passed. Port numbers must be within the range of 1-65565";
      break;
    case INFO_HOST:
      msg_buffer << "Host:";
      break;
    case INFO_SERVICE:
      msg_buffer << "Service:";
      break;
    case MESSAGE_SEND_FAILED:
      msg_buffer << "Failed to send message";
      break;
    case MESSAGE_RECV_FAILED:
      msg_buffer << "Failed to receive message";
      break;
    case MESSAGE_LOAD_FAILED:
      msg_buffer << "Failed to load message";
      break;
    case MESSAGE_DUMP_FAILED:
      msg_buffer << "Failed to dump message";
      break;
    case BUFFER_SEND_ERROR_TO_SERVICE:
      msg_buffer << "Failed to send buffer to service:";
      break;
    case BUFFER_SEND_ERROR_TO_HOST:
      msg_buffer << "Failed to send buffer to host:";
      break;
    case CONNECTED_TO_HOST:
      msg_buffer << "Connected to host:";
      break;
    case CONNECTED_TO_SERVICE:
      msg_buffer << "Connected to service:";
      break;
    case CONNECTED_FOR_ID:
      msg_buffer << "Connected to host for redirect id:";
      break;
    case CONNECTION_CLOSED:
      msg_buffer << "Connection has been closed:";
      break;
    case CONNECTION_CLOSED_BY_SERVICE:
      msg_buffer << "Connection has been closed by service:";
      break;
    case CONNECTION_CLOSED_BY_HOST:
      msg_buffer << "Connection has been closed by host:";
      break;
    case STREAM_PORT_OPENED:
      msg_buffer << "Service is now available at:";
      break;
    case NO_PORTS_AVAILABLE:
      msg_buffer << "Server has no ports available. Please try again later";
    case PROXYING_STARTED:
      msg_buffer << "Proxying started:";
      break;
    case PROXYING_ENDED:
      msg_buffer << "Proxying ended:";
      break;
    case AUTHENTICATION_REQUEST_SENT:
      msg_buffer << "Authentication request sent";
      break;
    case AUTHENTICATION_FAILED:
      msg_buffer << "Authentication failed. Now ending process";
      break;
    case AUTHENTICATION_SUCCESS:
      msg_buffer << "Authentication success";
      break;
    case SHA256_FAILED:
      msg_buffer << "Failed to hash token";
      break;
    case SERVER_DATABASE_ERROR:
      msg_buffer << "Server has returned an error";
      break;
    case HOST_RESPONSE_TIMEOUT:
      msg_buffer << "Host is not responsive. Timed out";
      break;
    case ENTER_TOKEN_INSTRUCTION:
      msg_buffer << "Please enter your token:";
      break;
    case INVALID_TOKEN:
      msg_buffer << "Invalid token format";
      break;
    case SIGNAL:
      msg_buffer << "Closing with signal";
      break;
    case DEBUG_MSG:
      msg_buffer << CYAN << "DEBUG_MSG:" << RESET;
      break;
  }

  if (detail != nullptr) {
    msg_buffer << ' ';
    msg_buffer << detail;
  }

  cout_buffer << msg_buffer.str() << ' ';

  if (verbose_level <= DEBUG) {
    cout_buffer << FAINT_GRAY;
    cout_buffer << '(';
    cout_buffer << function;
    cout_buffer << ')';
    cout_buffer << RESET;
  }
  cout_buffer << '\n';

  std::cout << cout_buffer.str();
}