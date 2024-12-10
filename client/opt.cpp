//
// Created by Jhean Lee on 2024/12/2.
//
#include <cstdlib>
#include <iostream>
#include <cstring>

#include <netdb.h>

#include "opt.hpp"

void print_help(const char *binary_name) {
  printf("Usage: %s [-H|--host-addr address ipv4-address] [-P|--host-port port] -s|--service-addr ipv4-address [-p|--service-port port]\n", binary_name);
}

const char *host = "0.0.0.0";
const char *readable_host = "0.0.0.0";
int host_main_port = 3000;
const char *local_service = "0.0.0.0";
int local_service_port = -1;
char hostname[NI_MAXHOST];

void opt_handler(int argc, char * const argv[]) {
  int opt;
  char *endptr;

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
      case 'h':
        print_help(argv[0]);
        exit(0);
      default:
        std::cerr << "[Error] Unknown flag. For help, please use --help (-h) flag.\n";
        exit(1);
    }
  }

  if (local_service_port == -1) {
    std::cerr << "[Error] Flag --service-port is not set.\n";
    exit(1);
  }

  std::cout << "[Info] Streaming host set to " << host << ':' << host_main_port << '\n';
  std::cout << "[Info] Local service set to " << local_service << ':' << local_service_port << '\n';
}