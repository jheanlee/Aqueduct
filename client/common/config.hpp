//
// Created by Jhean Lee on 2024/10/29.
//

#ifndef AQUEDUCT_CONFIG_HPP
  #define AQUEDUCT_CONFIG_HPP
  extern const char *host;
  extern const char *readable_host;
  extern int host_main_port;

  extern int timeout_session_millisec;
  extern int timeout_proxy_millisec;

  extern const char *local_service;
  extern int local_service_port;

  extern std::string token;
#endif //AQUEDUCT_CONFIG_HPP
