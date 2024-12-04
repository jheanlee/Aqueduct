//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_CONFIG_HPP
  #define TUNNEL_CONFIG_HPP
  #include <vector>
  #include <numeric>

  static const char *host = "0.0.0.0";
  static const int main_port = 3000;
  static const int connection_limit = 5;

  static const int proxy_port_begin = 51001;
  extern std::vector<int> proxy_port_available;
  void init_proxy_port_available();

  static const int first_message_timeout_sec = 30;
  static const int heartbeat_sleep_sec = 30;
  static const int heartbeat_timeout_sec = 30;

#endif //TUNNEL_CONFIG_HPP
