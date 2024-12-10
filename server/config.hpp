//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_CONFIG_HPP
  #define TUNNEL_CONFIG_HPP
  #include <queue>

  static const char *host = "0.0.0.0";
  extern int control_port;
  static const int connection_limit = 5;

  extern int proxy_port_start;
  extern int proxy_port_limit;
  extern std::queue<int> proxy_ports_available;
  void init_proxy_ports_available();

  static const int first_message_timeout_sec = 30;
  static const int heartbeat_sleep_sec = 30;
  static const int heartbeat_timeout_sec = 30;

#endif //TUNNEL_CONFIG_HPP
