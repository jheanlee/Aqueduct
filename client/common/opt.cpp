//
// Created by Jhean Lee on 2024/12/2.
//
#include <cstdlib>
#include <cstring>
#include <regex>

#include <netdb.h>

#include "opt.hpp"
#include "console.hpp"

void print_help() {
  printf("sphere-linked-client [OPTIONS]\n"
         "OPTIONS\n"
         "    -h, --help                          Prints this page\n"
         "    -H, --host-addr <ipv4|domain>       Sets host to <ipv4|domain>\n"
         "                                        Default is 0.0.0.0\n"
         "    -P, --host-port <port>              Uses host:<port> as control port (see --control-port of server)\n"
         "                                        Default is 3000\n"
         "    -s, --service-addr <ipv4>           Sets the address of service to be tunneled to <ipv4>\n"
         "                                        Default is 0.0.0.0\n"
         "    -p, --service-port <port>           Tunnels service:<port> to host\n"
         "                                        This option is REQUIRED\n"
         "    --session-select-timeout <time>     The time poll() waits each call when accepting connections, see `man poll` for more information\n"
         "                                        Default is 10\n"
         "    --proxy-select-timeout <time>       The time poll() waits each call during proxying, see `man poll` for more information\n"
         "                                        Default is 1\n"
         "    -t, --token <token>                 Token for accessing server\n"
         "                                        This option is REQUIRED\n"
  );

}

const char *host = "0.0.0.0";
const char *readable_host = "0.0.0.0";
int host_main_port = 30330;
const char *local_service = "0.0.0.0";
int local_service_port = -1;
int timeout_session_millisec = 10;
int timeout_proxy_millisec = 1;
char hostname[NI_MAXHOST];
std::regex ipv4("(\\d{1,3}(\\.\\d{1,3}){3})");
std::string token;
bool verbose = false;

void opt_handler(int argc, char * const argv[]) {
  int opt;
  char *endptr;
  int timeout = 0;

  struct addrinfo *result;
  struct addrinfo *addr_ptr;
  struct addrinfo hint;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_INET;
  int error = 0;

  while ((opt = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
    switch (opt) {
      case 'H':
        readable_host = optarg;
        if (std::regex_match(readable_host, ipv4)) {
          host = readable_host;
          break;
        }

        error = getaddrinfo(readable_host, nullptr, &hint, &result);
        if (error != 0) {
          console(ERROR, RESOLVE_HOST_FAILED, nullptr, "option");
          exit(EXIT_FAILURE);
        }
        addr_ptr = result;
        while (addr_ptr != nullptr) {
          error = getnameinfo(addr_ptr->ai_addr, addr_ptr->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
          if (error != 0) {
            console(ERROR, RESOLVE_HOST_FAILED, nullptr, "option");
            exit(EXIT_FAILURE);
          }
          if (*hostname != '\0') {
            host = hostname;
            break;
          }
          addr_ptr = addr_ptr->ai_next;
        }
        break;
      case 'P':
        host_main_port = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          console(ERROR, PORT_INVALID_CHARACTER, nullptr, "option");
          exit(EXIT_FAILURE);
        }
        if (host_main_port < 1 || host_main_port > 65535) {
          console(ERROR, PORT_INVALID_RANGE, nullptr, "option");
          exit(EXIT_FAILURE);
        }
        break;
      case 's':
        local_service = optarg;
        break;
      case 'p':
        local_service_port = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          console(ERROR, PORT_INVALID_CHARACTER, nullptr, "option");
          exit(EXIT_FAILURE);
        }
        if (local_service_port < 1 || local_service_port > 65535) {
          console(ERROR, PORT_INVALID_RANGE, nullptr, "option");
        }
        break;
      case Long_Opts::SESSION_TIMEOUT:
        timeout_session_millisec = std::strtol(optarg, &endptr, 10);
        break;
      case Long_Opts::PROXY_TIMEOUT:
        timeout_proxy_millisec = std::strtol(optarg, &endptr, 10);
        break;
      case 't':
        token = std::string(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'h':
        print_help();
        exit(EXIT_SUCCESS);
      default:
        console(ERROR, OPTION_UNKNOWN, nullptr, "option");
        exit(EXIT_FAILURE);
    }
  }

  if (local_service_port == -1) {
    console(ERROR, OPTION_SERVICE_PORT_NOT_SET, nullptr, "option");
    exit(EXIT_FAILURE);
  }
  if (token.empty()) {
    console(ERROR, OPTION_TOKEN_NOT_SET, nullptr, "option");
    exit(EXIT_FAILURE);
  }

  console(INFO, INFO_HOST, (std::string(readable_host) + '(' + std::string(host) + ')' + ':' + std::to_string(host_main_port)).c_str(), "option");
  console(INFO, INFO_SERVICE, (std::string(local_service) + ':' + std::to_string(local_service_port)).c_str(), "option");
}