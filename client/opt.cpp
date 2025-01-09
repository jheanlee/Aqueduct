//
// Created by Jhean Lee on 2024/12/2.
//
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <regex>

#include <netdb.h>

#include "opt.hpp"

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
         "    --session-select-timeout <time>     The time select() waits each call when accepting connections, see `man select` for more information\n"
         "                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> %% 1000)\n"
         "                                        Default is 10\n"
         "    --proxy-select-timeout <time>       The time select() waits each call during proxying, see `man select` for more information\n"
         "                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> %% 1000)\n"
         "                                        Default is 1\n");

}

const char *host = "0.0.0.0";
const char *readable_host = "0.0.0.0";
int host_main_port = 30330;
const char *local_service = "0.0.0.0";
int local_service_port = -1;
int select_timeout_session_sec = 0;
int select_timeout_session_millisec = 10;
int select_timeout_proxy_sec = 0;
int select_timeout_proxy_millisec = 1;
char hostname[NI_MAXHOST];
std::regex ipv4("(\\d{1,3}(\\.\\d{1,3}){3})");

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
          std::cerr << "[Error] Unable to resolve host address (flag --host-addr)\n";
          exit(1);
        }
        addr_ptr = result;
        while (addr_ptr != nullptr) {
          error = getnameinfo(addr_ptr->ai_addr, addr_ptr->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
          if (error != 0) {
            std::cerr << "[Error] Unable to resolve host address (flag --host-addr)";
            exit(1);
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
          std::cerr << "[Error] Invalid character found in host port (flag --host-port)\n";
          exit(1);
        }
        break;
      case 's':
        local_service = optarg;
        break;
      case 'p':
        local_service_port = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in service port (flag --service-port)\n";
          exit(1);
        }
        break;
      case Long_Opts::SESSION_SELECT_TIMEOUT:
        timeout = std::strtol(optarg, &endptr, 10);
        select_timeout_session_millisec = timeout % 1000;
        select_timeout_session_sec = timeout / 1000;
        break;
      case Long_Opts::PROXY_SELECT_TIMEOUT:
        timeout = std::strtol(optarg, &endptr, 10);
        select_timeout_proxy_millisec = timeout % 1000;
        select_timeout_proxy_sec = timeout / 1000;
        break;
      case 'h':
        print_help();
        exit(0);
      default:
        std::cerr << "[Error] Unknown flag. For help, please use --help (-h) flag.\n";
        exit(1);
    }
  }

  if (local_service_port == -1) {
    std::cerr << "[Error] Please specify the port of the service to stream with flag --service-port (-p).\n";
    exit(1);
  }

  std::cout << "[Info] Streaming host set to " << readable_host << '(' << host  << ')' << ':' << host_main_port << '\n';
  std::cout << "[Info] Local service set to " << local_service << ':' << local_service_port << '\n';
}