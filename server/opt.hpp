//
// Created by Jhean Lee on 2024/12/10.
//

#ifndef TUNNEL_OPT_HPP
  #define TUNNEL_OPT_HPP

  #include <getopt.h>

  static const char *short_options = "s:l:p:h";
  static struct option long_options[] = {
          {"port-start",   required_argument, nullptr, 's'},
          {"port-limit",   required_argument, nullptr, 'l'},
          {"control-port", required_argument, nullptr, 'p'},
          {"help",         no_argument,       nullptr, 'h'},
          {0,              0,                 0,       0},
  };

  void print_help(const char *binary_name);

  void opt_handler(int argc, char *const argv[]);

#endif //TUNNEL_OPT_HPP
