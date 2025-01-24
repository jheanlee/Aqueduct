//
// Created by Jhean Lee on 2024/12/10.
//

#ifndef TUNNEL_OPT_HPP
  #define TUNNEL_OPT_HPP

  #include <getopt.h>

  static const char *short_options = "s:l:p:k:c:d:Vh";
  enum Long_Opts {
    SESSION_SELECT_TIMEOUT = 256,
    PROXY_SELECT_TIMEOUT = 257,
  };
  static struct option long_options[] = {
          {"port-start",              required_argument,  nullptr, 's'},
          {"port-limit",              required_argument,  nullptr, 'l'},
          {"control-port",            required_argument,  nullptr, 'p'},
          {"tls-key",                 required_argument,  nullptr, 'k'},
          {"tls-cert",                required_argument,  nullptr, 'c'},
          {"session-select-timeout",  required_argument,  nullptr, Long_Opts::SESSION_SELECT_TIMEOUT},
          {"proxy-select-timeout",    required_argument,  nullptr, Long_Opts::PROXY_SELECT_TIMEOUT},
          {"database",                required_argument,  nullptr, 'd'},
          {"verbose",                 no_argument,        nullptr, 'V'},
          {"help",                    no_argument,        nullptr, 'h'},
          {nullptr,                   0,                  nullptr, 0},
  };

  void opt_handler(int argc, char *const argv[]);

#endif //TUNNEL_OPT_HPP
