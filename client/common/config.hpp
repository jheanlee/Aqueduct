//
// Created by Jhean Lee on 2024/10/29.
//

#ifndef TUNNEL_CONFIG_HPP
  #define TUNNEL_CONFIG_HPP
  extern const char *host;
  extern const char *readable_host;
  extern int host_main_port;

  extern int timeout_session_millisec;
  extern int timeout_proxy_millisec;

  extern const char *local_service;
  extern int local_service_port;

  extern std::string token;
#endif //TUNNEL_CONFIG_HPP
