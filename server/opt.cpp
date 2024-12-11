//
// Created by Jhean Lee on 2024/12/10.
//
#include <cstdlib>
#include <iostream>

#include "opt.hpp"
#include "config.hpp"

int control_port = 3000;
int proxy_port_start = 51000;
int proxy_port_limit = 200;

void print_help(const char *binary_name) {
  printf("sphere-linked-server [OPTIONS]\n"
         "Options\n"
         "    -h, --help                      Prints usage\n"
         "    -p, --control-port <port>       Client will connect to localhost:<port>\n"
         "                                    Should be identical with --host-port of client\n"
         "                                    Default is 3000\n"
         "    -s, --port-start <port>         The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)\n"
         "                                    Default is 51000\n"
         "    -l, --port-limit <count>        Proxy ports will have a limit of <count> ports\n"
         "                                    Default is 200\n");
}

void opt_handler(int argc, char * const argv[]) {
  int opt;
  char *endptr;

  while ((opt = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
    switch (opt) {
      case 's':
        proxy_port_start = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in starting port (flag --port-start)\n";
          exit(1);
        }
        break;
      case 'l':
        proxy_port_limit = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in port number limit (flag --port-limit)\n";
          exit(1);
        }
        break;
      case 'p':
        control_port = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in control port (flag --control-port)\n";
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

  std::cout << "[Info] Streaming host set to " << host << ':' << control_port << '\n';
}