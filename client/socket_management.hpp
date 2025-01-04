//
// Created by Jhean Lee on 2024/12/26.
//

#ifndef SPHERE_LINKED_SOCKET_MANAGEMENT_HPP
  #define SPHERE_LINKED_SOCKET_MANAGEMENT_HPP
#include <iostream>

  #include <openssl/ssl.h>
  #include <arpa/inet.h>

  #include "config.hpp"

  void init_openssl();
  void cleanup_openssl();
  SSL_CTX *create_context();

#endif //SPHERE_LINKED_SOCKET_MANAGEMENT_HPP
