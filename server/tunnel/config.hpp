//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_CONFIG_HPP
  #define TUNNEL_CONFIG_HPP
  #include <queue>
  #include <string>

  extern const char *cert_path;
  extern const char *key_path;

  static const char *host = "0.0.0.0";
  extern int ssl_control_port;
  static const int connection_limit = 5;

  extern int proxy_port_start;
  extern int proxy_port_limit;
  extern std::queue<int> proxy_ports_available;
  void init_proxy_ports_available();

  static const int first_message_timeout_sec = 30;
  static const int heartbeat_sleep_sec = 30;
  static const int heartbeat_timeout_sec = 30;

  extern int select_timeout_session_sec;
  extern int select_timeout_session_millisec;
  extern int select_timeout_proxy_sec;
  extern int select_timeout_proxy_millisec;

  extern const char *db_path;

#endif //TUNNEL_CONFIG_HPP
