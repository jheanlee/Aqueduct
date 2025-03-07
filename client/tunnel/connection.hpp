//
// Created by Jhean Lee on 2024/10/30.
//

#ifndef SPHERE_LINKED_CONNECTION_HPP
  #define SPHERE_LINKED_CONNECTION_HPP

  #include <atomic>
  #include <queue>
  #include <string>

  #include <arpa/inet.h>
  #include <openssl/ssl.h>

  void send_heartbeat_message(SSL *server_ssl, char *buffer);
  void send_auth_message(SSL *server_ssl, char *buffer, size_t buffer_size);
  void service_thread_func(std::atomic<bool> &flag_kill, std::queue<std::string> &user_id);
  void proxy_thread_func(std::atomic<bool> &flag_kill, SSL *host_ssl, int host_fd, std::string redirect_id, int service_fd);

#endif //SPHERE_LINKED_CONNECTION_HPP