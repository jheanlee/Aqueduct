//
// Created by Jhean Lee on 2024/12/26.
//

#include <iostream>

#include "socket_management.hpp"

void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
  EVP_cleanup();
}

SSL_CTX *create_context() {
  const SSL_METHOD *method = TLS_server_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) {
    std::cerr << "[Error] Failed to create SSL context \033[2;90m(socket_management)\033[0m\n"; exit(EXIT_FAILURE);
  }
  return ctx;
}

void config_context(SSL_CTX *ctx) {
  if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0 || SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
    std::cerr << "[Error] Failed to load certificate or key \033[2;90m(socket_management)\033[0m\n"; exit(EXIT_FAILURE);
  }
}

int bind_socket(int &socket_fd, sockaddr_in &addr) {
  int on = 1;
  if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    return -1;
  }
  if (listen(socket_fd, connection_limit) == -1) {
    return -2;
  }
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) {
    return -3;
  }
  return 0;
}

int create_socket(sockaddr_in &addr) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tcp
  if (socket_fd == -1) {
    std::cerr << "[Error] Failed to create socket \033[2;90m(socket_management)\033[0m\n"; exit(EXIT_FAILURE);
  }

  int status = bind_socket(socket_fd, addr);
  switch (status) {
    case -1:
      std::cerr << "[Error] Binding error \033[2;90m(socket_management)\033[0m\n";
      exit(EXIT_FAILURE);
    case -2:
      std::cerr << "[Error] Listening error \033[2;90m(socket_management)\033[0m\n";
      exit(EXIT_FAILURE);
    case -3:
      std::cerr << "[Error] Setsockopt error \033[2;90m(socket_management)\033[0m\n";
      exit(EXIT_FAILURE);
    default:
      break;
  }

  return socket_fd;
}