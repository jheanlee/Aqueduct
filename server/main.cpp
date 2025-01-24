//
// Created by Jhean Lee on 2024/10/2.
//

#include <atomic>
#include <thread>

#include <sqlite3.h>

#include "tunnel/config.hpp"
#include "tunnel/connection.hpp"
#include "tunnel/opt.hpp"
#include "tunnel/socket_management.hpp"
#include "tunnel/shared.hpp"
#include "database/auth.hpp"

sqlite3 *shared_resources::db = nullptr;
int main(int argc, char *argv[]) {
  opt_handler(argc, argv);
  init_proxy_ports_available();
  init_openssl();

  open_db(&shared_resources::db);
  create_sqlite_functions(shared_resources::db);
  check_tables(shared_resources::db);

  std::atomic<bool> flag_kill = false;
  std::thread control_thread;
  std::thread ssl_control_thread;

  ssl_control_thread = std::thread(ssl_control_thread_func, std::ref(flag_kill));

  ssl_control_thread.join();

  sqlite3_close(shared_resources::db);
  return 0;
}