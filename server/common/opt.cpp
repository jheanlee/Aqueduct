//
// Created by Jhean Lee on 2024/12/10.
//
#include <cstdlib>
#include <cstring>
#include <string>

#include "opt.hpp"
#include "shared.hpp"
#include "console.hpp"

int ssl_control_port = 30330;
int proxy_port_start = 51000;
int proxy_port_limit = 200;
int select_timeout_session_sec = 0;
int select_timeout_session_millisec = 10;
int select_timeout_proxy_sec = 0;
int select_timeout_proxy_millisec = 1;
const char *cert_path = "\0";
const char *key_path = "\0";
const char *db_path = "./sphere-linked.sqlite";
bool verbose = false;

static void print_help() {
  printf("sphere-linked-server [OPTIONS]\n"
         "OPTIONS\n"
         "    -h, --help                          Prints this page\n"
         "    -V, --verbose                       Outputs detailed information\n"
         "    -p, --control-port <port>           Client will connect to localhost:<port>\n"
         "                                        Should be identical with --host-port of client\n"
         "                                        Default is 30330\n"
         "    -s, --port-start <port>             The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)\n"
         "                                        Default is 51000\n"
         "    -l, --port-limit <count>            Proxy ports will have a limit of <count> ports\n"
         "                                        Default is 200\n"
         "    -k, --tls-key <path>                The path to a private key file used for TLS/SSL encryption\n"
         "                                        This option is REQUIRED\n"
         "    -c, --tls-cert <path>               The path to a certification file used for TLS/SSL encryption\n"
         "                                        This certification must match the key\n"
         "                                        This option is REQUIRED\n"
         "    --session-select-timeout <time>     The time select() waits each call when accepting connections, see `man select` for more information\n"
         "                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> %% 1000)\n"
         "                                        Default is 10\n"
         "    --proxy-select-timeout <time>       The time select() waits each call during proxying, see `man select` for more information\n"
         "                                        timeval.sec would be (<time> / 1000), and timeval.usec would be (<time> %% 1000)\n"
         "                                        Default is 1\n"
         "    -d, --database <path>               The path to database file\n"
         "                                        Default is ./sphere-linked.sqlite\n");
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
          console(ERROR, PORT_INVALID_CHARACTER, nullptr, "opt::opt_handler");
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start <= 0 || proxy_port_start > 65535) {
          console(ERROR, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start < 1024) {
          console(WARNING, PORT_WELL_KNOWN, nullptr, "opt::opt_handler");
        }
        break;
      case 'l':
        proxy_port_limit = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          console(ERROR, PORT_INVALID_CHARACTER, nullptr, "opt::opt_handler");
          exit(EXIT_FAILURE);
        }
        if (proxy_port_limit < 1) {
          console(ERROR, PORT_INVALID_LIMIT, nullptr, "opt::opt_handler");
          exit(EXIT_FAILURE);
        }
        if (proxy_port_start + proxy_port_limit - 1 > 65535) {
          console(WARNING, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
        }
        break;
      case 'p':
        ssl_control_port = std::strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          console(ERROR, PORT_INVALID_CHARACTER, nullptr, "opt::opt_handler");
          exit(EXIT_FAILURE);
        }
        if (ssl_control_port <= 0 || ssl_control_port > 65535) {
          console(ERROR, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
          exit(EXIT_FAILURE);
        }
        if (ssl_control_port < 1024) {
          console(WARNING, PORT_WELL_KNOWN, nullptr, "opt::opt_handler");
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
      case 'd':
        db_path = optarg;
        break;
      case 'V':
        verbose = true;
        break;
      case 'h':
        print_help();
        exit(EXIT_SUCCESS);
      default:
        console(ERROR, OPTION_UNKNOWN, nullptr, "opt::opt_handler");
        exit(EXIT_FAILURE);
    }
  }

  if (strlen(key_path) == 0 || key_path[0] == '\0') {
    console(ERROR, OPTION_KEY_NOT_SET, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }
  if (strlen(cert_path) == 0 || cert_path[0] == '\0') {
    console(ERROR, OPTION_CERT_NOT_SET, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }

  console(INFO, INFO_CERT_PATH, cert_path, "opt::opt_handler");
  console(INFO, INFO_KEY_PATH, key_path, "opt::opt_handler");
  console(INFO, INFO_DB_PATH, db_path, "opt::opt_handler");
  console(INFO, INFO_HOST, (std::string(host) + ':' + std::to_string(ssl_control_port)).c_str(), "opt::opt_handler");
}