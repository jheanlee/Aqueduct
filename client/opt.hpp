//
// Created by Jhean Lee on 2024/12/2.
//

#ifndef TUNNEL_OPT_HPP
  #define TUNNEL_OPT_HPP
  #include <cstdlib>
  #include <iostream>

  #include <unistd.h>
  #include <getopt.h>

  #include "config.hpp"

  static const char* short_options = "H:P:s:p:h";
  static struct option long_options[] = {
          {"host-addr", required_argument, nullptr, 'H'},
          {"host-port", required_argument, nullptr, 'P'},
          {"service-addr", required_argument, nullptr, 's'},
          {"service-port", required_argument, nullptr, 'p'},
          {"help", no_argument, nullptr, 'h'},
          {0, 0, 0, 0},
  };

  void print_help(const char *binary_name);
  void opt_handler(int argc, char * const argv[]);

#endif //TUNNEL_OPT_HPP
