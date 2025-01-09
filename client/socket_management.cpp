//
// Created by Jhean Lee on 2024/12/26.
//

#include "socket_management.hpp"

void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
  EVP_cleanup();
}

SSL_CTX *create_context() {
  const SSL_METHOD *method = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) {
    std::cerr << "[Error] Unable to create SSL context \033[2;90m(socket_management)\033[0m\n"; exit(EXIT_FAILURE);
  }
  return ctx;
}