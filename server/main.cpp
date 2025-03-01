//
// Created by Jhean Lee on 2024/10/2.
//

#include <thread>
#include <chrono>

#include "tunnel/connection.hpp"
#include "common/opt.hpp"
#include "tunnel/socket_management.hpp"
#include "common/shared.hpp"
#include "database/database.hpp"
#include "common/signal_handler.hpp"
#include "database/client.hpp"
#include "common/input.hpp"
#include "database/authentication.hpp"

int main(int argc, char *argv[]) {
  register_signal();
  shared_resources::process_start = std::chrono::system_clock::now();
  opt_handler(argc, argv);
  init_proxy_ports_available();
  init_openssl();

  open_db(&shared_resources::db);
  create_sqlite_functions(shared_resources::db);
  check_tables(shared_resources::db);
  check_token_expiry();

  std::thread ssl_control_thread(ssl_control_thread_func);
  std::thread update_client_db_thread(update_client_db_thread_func);
  std::thread input_thread(input_thread_func);
  ssl_control_thread.join();
  update_client_db_thread.join();
  input_thread.join();

  if (!shared_resources::flag_handling_signal) signal_handler(0);
  return 0;
}