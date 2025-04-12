//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef AQUEDUCT_CONNECTION_HPP
  #define AQUEDUCT_CONNECTION_HPP

  #include <atomic>
  #include <unordered_map>
  #include <string>
  #include <mutex>

  #include <arpa/inet.h>
  #include <openssl/ssl.h>
  #include <sqlite3.h>

  #include "../common/shared.hpp"

  void ssl_control_thread_func();
  void ssl_session_thread_func(int client_fd, SSL *client_ssl, sockaddr_in client_addr);
  void ssl_heartbeat_thread_func(SSL *client_ssl, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received,
                                 std::atomic<bool> &flag_kill, std::mutex &send_mutex);
  void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::atomic<bool> &flag_auth_received, std::string &auth,
                                      SSL *client_ssl, sockaddr_in &client_addr, Client &client, std::mutex &send_mutex);
  void proxy_thread_func(SSL *client_ssl, External_User external_user, std::atomic<bool> &flag_kill, Client &client, ClientData &client_data);

#endif //AQUEDUCT_CONNECTION_HPP