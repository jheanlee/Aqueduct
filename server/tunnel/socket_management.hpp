//
// Created by Jhean Lee on 2024/12/26.
//

#ifndef SPHERE_LINKED_SOCKET_MANAGEMENT_HPP
  #define SPHERE_LINKED_SOCKET_MANAGEMENT_HPP

  #include <openssl/ssl.h>
  #include <arpa/inet.h>

  #include "config.hpp"

  void init_openssl();
  void cleanup_openssl();
  SSL_CTX *create_context();
  void config_context(SSL_CTX *ctx);

  int bind_socket(int &socket_fd, sockaddr_in &addr);
  int create_socket(sockaddr_in &addr);

#endif //SPHERE_LINKED_SOCKET_MANAGEMENT_HPP