//
// Created by Jhean Lee on 2024/12/10.
//
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "opt.hpp"
#include "config.hpp"

int ssl_control_port = 30330;
int proxy_port_start = 51000;
int proxy_port_limit = 200;
int select_timeout_session_sec = 0;
int select_timeout_session_millisec = 10;
int select_timeout_proxy_sec = 0;
int select_timeout_proxy_millisec = 1;
const char *cert_path = "\0";
const char *key_path = "\0";

void print_help() {  //  TODO: update
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
  long timeout = 0;

  while ((opt = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
    switch (opt) {
      case 's':
        proxy_port_start = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in starting port (flag --port-start)\n";
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start <= 0 || proxy_port_start > 65535) {
          std::cerr << "[Error] Invalid starting port value (flag --port-start)\n";
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start < 1024) {
          std::cerr << "[Warning] Well-known ports (range 0-1023) passed to starting port (flag --port-start). These ports are typically restricted by system\n";
        }
        break;
      case 'l':
        proxy_port_limit = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in port number limit (flag --port-limit)\n";
          exit(EXIT_FAILURE);
        }
        if (proxy_port_limit < 1) {
          std::cerr << "[Error] Invalid port limit (flag --port-limit)\n";
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start + proxy_port_limit - 1 > 65535) {
          std::cerr << "[Warning] Proxy port numbers may exceed port range (65535) (flag --port-limit)\n";
        }
        break;
      case 'p':
        ssl_control_port = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          std::cerr << "[Error] Invalid character found in control port (flag --control-port)\n";
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start <= 0 || proxy_port_start > 65535) {
          std::cerr << "[Error] Invalid starting port value (flag --control-port)\n";
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start < 1024) {
          std::cerr << "[Warning] Well-known ports (range 0-1023) passed to starting port (flag --control-port). These ports are typically restricted by system\n";
        }
        break;
      case 'k':
        key_path = optarg;
        break;
      case 'c':
        cert_path = optarg;
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
        exit(EXIT_SUCCESS);
      default:
        std::cerr << "[Error] Unknown flag. For help, please use the --help (-h) flag.\n";
        exit(EXIT_FAILURE);
    }
  }

  if (strlen(key_path) == 0 || key_path[0] == '\0') {
    std::cerr << "[Error] Please specify your private key path with the --tls-key (-k) flag\n";
    exit(EXIT_FAILURE);
  }
  if (strlen(cert_path) == 0 || cert_path[0] == '\0') {
    std::cerr << "[Error] Please specify your certificate path with the --tls-cert (-c) flag\n";
    exit(EXIT_FAILURE);
  }

  std::cout << "[Info] TLS private key set to " << key_path << '\n';
  std::cout << "[Info] TLS certificate set to " << cert_path << '\n';
  std::cout << "[Info] Streaming host set to " << host << ':' << ssl_control_port << '\n';
}