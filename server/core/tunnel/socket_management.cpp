//
// Created by Jhean Lee on 2024/12/26.
//

#include <iostream>

#include "socket_management.hpp"
#include "../common/shared.hpp"
#include "../common/console.hpp"
#include "../common/signal_handler.hpp"

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
    console(ERROR, SSL_CREATE_CONTEXT_FAILED, nullptr, "socket_management::create_context");
    signal_handler(EXIT_FAILURE);
  }
  return ctx;
}

void config_context(SSL_CTX *ctx) {
  if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0 || SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
    console(ERROR, SSL_LOAD_CERT_KEY_FAILED, nullptr, "socket_management::config_context");
    signal_handler(EXIT_FAILURE);
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
    console(ERROR, SOCK_CREATE_FAILED, nullptr, "socket_management::create_socket");
    signal_handler(EXIT_FAILURE);
  }

  int status = bind_socket(socket_fd, addr);
  switch (status) {
    case -1:
      console(ERROR, SOCK_BIND_FAILED, nullptr, "socket_management::create_socket");
      signal_handler(EXIT_FAILURE);
    case -2:
      console(ERROR, SOCK_LISTEN_FAILED, nullptr, "socket_management::create_socket");
      signal_handler(EXIT_FAILURE);
    case -3:
      console(ERROR, SOCK_SETSOCKOPT_FAILED, nullptr, "socket_management::create_socket");
      signal_handler(EXIT_FAILURE);
    default:
      break;
  }

  return socket_fd;
}

void init_proxy_ports_available() {
  for (int i = proxy_port_start; i < proxy_port_start + proxy_port_limit; i++) {
    proxy_ports_available.push(i);
  }
}