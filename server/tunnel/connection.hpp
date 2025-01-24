//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_CONNECTION_HPP
  #define TUNNEL_CONNECTION_HPP

  #include <atomic>
  #include <unordered_map>
  #include <string>

  #include <arpa/inet.h>
  #include <openssl/ssl.h>
  #include <sqlite3.h>

void ssl_control_thread_func(std::atomic<bool> &flag_kill);
  void ssl_heartbeat_thread_func(SSL *client_ssl, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill);
  void ssl_session_thread_func(int client_fd, SSL *client_ssl, sockaddr_in client_addr, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id);
  void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::atomic<bool> &flag_auth_received, std::string &auth, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id, SSL *client_ssl, sockaddr_in &client_addr);
  void proxy_thread_func(SSL *client_ssl, int external_user_fd, sockaddr_in client_addr, std::atomic<bool> &flag_kill);

#endif //TUNNEL_CONNECTION_HPP