//
// Created by Jhean Lee on 2025/1/24.
//
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <mutex>

#if defined(__clang__) && defined(__APPLE__)
  #include <os/log.h>
#else
  #include <sys/syslog.h>
#endif


#include "console.hpp"
#include "shared.hpp"

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define YELLOW      "\033[33m"
#define FAINT_GRAY  "\033[2;90m"
#define CYAN        "\033[36m"

static int to_int(Level level) {
  switch (level) {
    case CRITICAL:
      return 50;
    case ERROR:
      return 40;
    case WARNING:
    case NOTICE:
      return 30;
    case INFO:
      return 20;
    case DEBUG:
      return 10;
    case INSTRUCTION:
      return -1;
  }
  return -1;
}

void console(Level level, Code code, const char *detail, const std::string &function) {
  if (to_int(level) < verbose_level && level != INSTRUCTION) return;

  std::ostringstream cout_buffer, msg_buffer;

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
    case NOTICE:
    case INFO:
      cout_buffer << "[Info] ";
      break;
    case DEBUG:
      cout_buffer << "[DEBUG] ";
      break;
    case INSTRUCTION:
      break;
  }
  cout_buffer << RESET;

  //  code
  switch (code) {
    case API_SOCK_CREATE_FAILED:
      msg_buffer << "Failed to create socket, api and webui services will not be available";
      break;
    case API_SOCK_BIND_FAILED:
      msg_buffer << "Failed to bind socket, api and webui services will not be available";
      break;
    case API_SOCK_LISTEN_FAILED:
      msg_buffer << "Failed to listen on socket, api and webui services will not be available";
      break;
    case API_SOCK_ACCEPT_FAILED:
      msg_buffer << "Failed to accept on socket, api and webui services may not be available";
      break;
    case API_SOCK_POLL_ERR:
      msg_buffer << "An error has been returned by poll(), api and webui services may not be available. errno:";
      break;
    case API_LISTEN_STARTED:
      msg_buffer << "API service has started";
      break;
    case API_SERVICE_ENDED:
      msg_buffer << "API service has ended";
      break;
    case API_CLIENT_CONNECTION_ACCEPTED:
      msg_buffer << "API connection accepted";
      break;
    case API_CONNECTION_CLOSED:
      msg_buffer << "API connection closed";
      break;
    case API_HEARTBEAT_TIMEOUT:
      msg_buffer << "API clienct heartbeat timed out";
      break;
    case API_START_PROCESS_FAILED:
      msg_buffer << "Failed to run API process, errno:";
      break;
    case API_PROCESS_STARTED:
      msg_buffer << "API process has been successfully started";
      break;
    case API_PROCESS_ENDED:
      msg_buffer << "API process has ended with exit code";
      break;
    case SOCK_CREATE_FAILED:
      msg_buffer << "Failed to create socket";
      break;
    case SOCK_BIND_FAILED:
      msg_buffer << "Failed to bind socket";
      break;
    case SOCK_LISTEN_FAILED:
      msg_buffer << "Failed to listen on socket";
      break;
    case SOCK_ACCEPT_FAILED:
      msg_buffer << "Failed to accept on socket";
      break;
    case SOCK_SETSOCKOPT_FAILED:
      msg_buffer << "Failed to set socket options";
      break;
    case SOCK_POLL_ERR:
      msg_buffer << "An error has been returned by poll(), errno:";
      break;
    case SSL_CREATE_CONTEXT_FAILED:
      msg_buffer << "Failed to create SSL context";
      break;
    case SSL_ACCEPT_FAILED:
      msg_buffer << "Failed to accept SSL connection";
      break;
    case SSL_LOAD_CERT_KEY_FAILED:
      msg_buffer << "Failed to load SSL certificate or key";
      break;
    case SSL_INIT_FAILED:
      msg_buffer << "Failed to initialise key/certificate generation";
      break;
    case SSL_BIO_FAILED:
      msg_buffer << "Failed to initialise key/certificate's BIO";
      break;
    case SSL_RSA_FAILED:
      msg_buffer << "Failed to generate RSA key";
      break;
    case SSL_KEY_WRITE_FAILED:
      msg_buffer << "Failed to write key into file";
      break;
    case SSL_CERT_WRITE_FAILED:
      msg_buffer << "Failed to write certificate into file";
      break;
    case SSL_CERT_SIGN_FAILED:
      msg_buffer << "Failed to sign certificate";
      break;
    case SQLITE_OPEN_FAILED:
      msg_buffer << "Failed to open SQLite database:";
      break;
    case SQLITE_CREATE_TABLE_FAILED:
      msg_buffer << "Failed to create SQLite table:";
      break;
    case SQLITE_PREPARE_FAILED:
      msg_buffer << "Failed to prepare SQL statement:";
      break;
    case SQLITE_BIND_PARAMETER_FAILED:
      msg_buffer << "Failed to bind SQL parameter:";
      break;
    case SQLITE_STEP_FAILED:
      msg_buffer << "Failed to execute SQL statement:";
      break;
    case SQLITE_RETRIEVE_FAILED:
      msg_buffer << "Failed to retrieve data from database:";
      break;
    case SQLITE_CLOSING:
      msg_buffer << "Closing SQLite database";
      break;
    case SQLITE_CLOSE_SUCCESS:
      msg_buffer << "Successfully closed SQLite database";
      break;
    case SQLITE_CLOSE_FAILED:
      msg_buffer << "Failed to close SQLite database:";
      break;
    case INVALID_DB:
      msg_buffer << "Invalid database pointer";
      break;
    case GENERATED_TOKEN:
      msg_buffer << "A new token has been generated for";
      break;
    case REMOVED_TOKEN:
      msg_buffer << "Token removed successfully";
      break;
    case SHA256_INIT_CONTEXT_FAILED:
      msg_buffer << "Failed to initialise SHA256 context";
      break;
    case SHA256_SET_CONTEXT_FAILED:
      msg_buffer << "Failed to set SHA256 context";
      break;
    case SHA256_UPDATE_CONTEXT_FAILED:
      msg_buffer << "Failed to update SHA256 context";
      break;
    case SHA256_FINALISE_CONTEXT_FAILED:
      msg_buffer << "Failed to finalise SHA256 context";
      break;
    case RAND_FAILED:
      msg_buffer << "Failed to generate random bytes";
      break;
    case OPTION_UNKNOWN:
      msg_buffer << "Unknown option passed to program. Please use the --help option to see usage";
      break;
    case OPTION_KEY_NOT_SET:
      msg_buffer << "Key for TLS connection is not set. Please specify the path to the private key using the --tls-key option";
      break;
    case OPTION_CERT_NOT_SET:
      msg_buffer << "Certificate for TLS connection is not set. Please specify the path to the certificate using the --tls-cert option";
      break;
    case PORT_INVALID_CHARACTER:
      msg_buffer << "Invalid value passed as port number";
      break;
    case PORT_INVALID_RANGE:
      msg_buffer << "Invalid port range passed. Port numbers must be within the range of 1-65565";
      break;
    case PORT_WELL_KNOWN:
      msg_buffer << "Well-known ports (range 1-1023) passed. May require escalated privilages to bind";
      break;
    case PORT_INVALID_LIMIT:
      msg_buffer << "Invalid port limit";
      break;
    case PORT_MAY_EXCEED:
      msg_buffer << "Port numbers may exceed 65535";
      break;
    case INFO_SSL_KEY_PATH:
      msg_buffer << "SSL private key:";
      break;
    case INFO_SSL_CERT_PATH:
      msg_buffer << "SSL certificate:";
      break;
    case INFO_JWT_PRIVKEY_PATH:
      msg_buffer << "JWT private key:";
      break;
    case INFO_JWT_PUBKEY_PATH:
      msg_buffer << "JWT public key:";
      break;
    case INFO_DB_PATH:
      msg_buffer << "Database file:";
      break;
    case INFO_HOST:
      msg_buffer << "Streaming host:";
      break;
    case MESSAGE_SEND_FAILED:
      msg_buffer << "Failed to send message";
      break;
    case MESSAGE_LOAD_FAILED:
      msg_buffer << "Failed to load message";
      break;
    case MESSAGE_DUMP_FAILED:
      msg_buffer << "Failed to dump message";
      break;
    case BUFFER_SEND_ERROR_TO_CLIENT:
      msg_buffer << "Failed to send buffer to client:";
      break;
    case BUFFER_SEND_ERROR_TO_EXTERNAL_USER:
      msg_buffer << "Failed to send buffer to external user:";
      break;
    case CONNECTION_LISTEN_STARTED:
      msg_buffer << "Listening for connection";
      break;
    case TUNNEL_SERVICE_ENDED:
      msg_buffer << "Tunnel service has ended";
      break;
    case CLIENT_CONNECTION_ACCEPTED:
      msg_buffer << "Accepted connection from client:";
      break;
    case EXTERNAL_CONNECTION_ACCEPTED:
      msg_buffer << "Accepted external connection:";
      break;
    case CONNECTION_CLOSED:
      msg_buffer << "Connection has been closed:";
      break;
    case CONNECTION_CLOSED_BY_CLIENT:
      msg_buffer << "Proxy connection has been closed by client:";
      break;
    case CONNECTION_CLOSED_BY_EXTERNAL_USER:
      msg_buffer << "Proxy connection has been closed by external user";
      break;
    case HEARTBEAT_TIMEOUT:
      msg_buffer << "Client heartbeat timed out:";
      break;
    case AUTHENTICATION_FAILED:
      msg_buffer << "Client authentication failed:";
      break;
    case AUTHENTICATION_SUCCESS:
      msg_buffer << "Client authentication success:";
      break;
    case PROXY_PORT_NEW:
      msg_buffer << "Opened new proxy port:";
      break;
    case PROXYING_STARTED:
      msg_buffer << "Proxying started:";
      break;
    case PROXYING_ENDED:
      msg_buffer << "Proxying ended:";
      break;
    case NO_PORT_AVAILABLE:
      msg_buffer << "No available ports";
      break;
    case CREATE_DIR_FAILED:
      msg_buffer << "Unable to create directory";
      break;
    case USER_CREATED:
      msg_buffer << "User created:";
      break;
    case USER_MODIFIED:
      msg_buffer << "User modified:";
      break;
    case USER_REMOVED:
      msg_buffer << "User removed:";
      break;
    case USERNAME_INVALID:
      msg_buffer << "Invalid username. Aborting";
      break;
    case PASSWORD_INVALID:
      msg_buffer << "Invalid password. Aborting";
      break;
    case USERNAME_USED:
      msg_buffer << "Username already exists";
      break;
    case USERNAME_NOT_FOUND:
      msg_buffer << "Username not found";
      break;
    case UNKNOWN_OPTION:
      msg_buffer << "Unknown option. Aborting";
      break;
    case USERNAME_NEW_INSTRUCTION:
      msg_buffer << "Please enter the username of the new user:";
      break;
    case USERNAME_MODIFY_INSTRUCTION:
      msg_buffer << "Please enter the username of the user to modify:";
      break;
    case USERNAME_REMOVE_INSTRUCTION:
      msg_buffer << "Please enter the username of the user to REMOVE:";
      break;
    case PASSWORD_NEW_INSTRUCTION:
      msg_buffer << "Please set a password for this user:";
      break;
    case REMOVE_DOUBLE_CHECK_INSTRUCTION:
      msg_buffer << "Enter the username again to confirm:";
      break;
    case SIGNAL:
      msg_buffer << "Closing with signal";
      break;
    case DEBUG_MSG:
      cout_buffer << CYAN << "DEBUG_MSG:" << RESET;
      break;
  }

  if (detail != nullptr) {
    msg_buffer << ' ';
    msg_buffer << detail;
  }

  cout_buffer << msg_buffer.str() << ' ';

  if (verbose_level <= to_int(DEBUG)) {
    cout_buffer << FAINT_GRAY;
    cout_buffer << "(CORE::";
    cout_buffer << function;
    cout_buffer << ')';
    cout_buffer << RESET;
  }
  cout_buffer << '\n';

  if (shared_resources::daemon_mode && to_int(level) >= std::max(verbose_level, to_int(WARNING))) {
    #if defined(__OS_LOG_H__)
      switch (level) {
        case CRITICAL:
          os_log_with_type(shared_resources::os_log_aqueduct, OS_LOG_TYPE_FAULT, "%{public}s", msg_buffer.str().c_str());
        case ERROR:
          os_log_with_type(shared_resources::os_log_aqueduct, OS_LOG_TYPE_ERROR, "%{public}s", msg_buffer.str().c_str());
          break;
        case WARNING:
          os_log_with_type(shared_resources::os_log_aqueduct, OS_LOG_TYPE_DEFAULT, "%{public}s", msg_buffer.str().c_str());
          break;
        case NOTICE:
          os_log_with_type(shared_resources::os_log_aqueduct, OS_LOG_TYPE_DEFAULT, "%{public}s", msg_buffer.str().c_str());
          break;
        case INFO:
          os_log_with_type(shared_resources::os_log_aqueduct, OS_LOG_TYPE_INFO, "%{public}s", msg_buffer.str().c_str());
          break;
        case DEBUG:
          os_log_with_type(shared_resources::os_log_aqueduct, OS_LOG_TYPE_DEBUG, "%{public}s", msg_buffer.str().c_str());
          break;
        case INSTRUCTION:
          break;
      }
    #else
      switch (level) {
        case CRITICAL:
          syslog(LOG_CRIT, "%s", msg_buffer.str().c_str());
        case ERROR:
          syslog(LOG_ERR, "%s", msg_buffer.str().c_str());
          break;
        case WARNING:
          syslog(LOG_WARNING, "%s", msg_buffer.str().c_str());
          break;
        case NOTICE:
          syslog(LOG_NOTICE, "%s", msg_buffer.str().c_str());
          break;
        case INFO:
          syslog(LOG_INFO, "%s", msg_buffer.str().c_str());
          break;
        case DEBUG:
          syslog(LOG_DEBUG, "%s", msg_buffer.str().c_str());
          break;
        case INSTRUCTIOn:
          break;
      }
    #endif
  } else if (!shared_resources::daemon_mode) {
    std::lock_guard<std::mutex> cout_lock(shared_resources::cout_mutex);
    std::cout << cout_buffer.str();

  }
}
