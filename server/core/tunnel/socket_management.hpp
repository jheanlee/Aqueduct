//
// Created by Jhean Lee on 2024/12/26.
//

#ifndef AQUEDUCT_SOCKET_MANAGEMENT_HPP
  #define AQUEDUCT_SOCKET_MANAGEMENT_HPP

  #include <openssl/ssl.h>
  #include <arpa/inet.h>

  void init_openssl();
  void cleanup_openssl();
  SSL_CTX *create_context();
  void config_context(SSL_CTX *ctx);

  int bind_socket(int &socket_fd, sockaddr_in &addr);
  int create_socket(sockaddr_in &addr);

  void init_proxy_ports_available();

#endif //AQUEDUCT_SOCKET_MANAGEMENT_HPP