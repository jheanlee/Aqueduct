//
// Created by Jhean Lee on 2025/2/14.
//

#ifndef SPHERE_LINKED_CLIENT_HPP
  #define SPHERE_LINKED_CLIENT_HPP
  #include <sqlite3.h>

  #include "../common/shared.hpp"

  int update_client_db(ClientData &client);
  void update_client_db_thread_func();
  void list_clients();

#endif //SPHERE_LINKED_CLIENT_HPP
