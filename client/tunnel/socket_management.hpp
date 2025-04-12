//
// Created by Jhean Lee on 2024/12/26.
//

#ifndef AQUEDUCT_SOCKET_MANAGEMENT_HPP
  #define AQUEDUCT_SOCKET_MANAGEMENT_HPP
  #include <iostream>

  #include <openssl/ssl.h>

  #include "../common/config.hpp"

  void init_openssl();
  void cleanup_openssl();
  SSL_CTX *create_context();

#endif //AQUEDUCT_SOCKET_MANAGEMENT_HPP
