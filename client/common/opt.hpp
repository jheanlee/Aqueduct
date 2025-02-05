//
// Created by Jhean Lee on 2024/12/2.
//

#ifndef TUNNEL_OPT_HPP
  #define TUNNEL_OPT_HPP

  #include <getopt.h>

  static const char *short_options = "H:P:s:p:t:vh";
  enum Long_Opts {
    SESSION_TIMEOUT = 256,
    PROXY_TIMEOUT = 257,
  };
  static struct option long_options[] = {
          {"host-addr",               required_argument, nullptr, 'H'},
          {"host-port",               required_argument, nullptr, 'P'},
          {"service-addr",            required_argument, nullptr, 's'},
          {"service-port",            required_argument, nullptr, 'p'},
          {"session-select-timeout",  required_argument, nullptr, Long_Opts::SESSION_TIMEOUT},
          {"proxy-select-timeout",    required_argument, nullptr, Long_Opts::PROXY_TIMEOUT},
          {"token",                   required_argument, nullptr, 't'},
          {"verbose",                 no_argument,       nullptr, 'v'},
          {"help",                    no_argument,       nullptr, 'h'},
          {nullptr,                   0,                 nullptr, 0},
  };

  void print_help();
  void opt_handler(int argc, char *const argv[]);

#endif //TUNNEL_OPT_HPP
