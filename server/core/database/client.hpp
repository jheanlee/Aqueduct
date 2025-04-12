//
// Created by Jhean Lee on 2025/2/14.
//

#ifndef AQUEDUCT_CLIENT_HPP
  #define AQUEDUCT_CLIENT_HPP
  #include <sqlite3.h>

  #include "../common/shared.hpp"

  int update_client_db(ClientData &client);
  void update_client_db_thread_func();
  void update_client_copy();
  void list_clients();

#endif //AQUEDUCT_CLIENT_HPP
