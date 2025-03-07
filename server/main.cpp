//
// Created by Jhean Lee on 2024/10/2.
//

#include <cstdio>
#include <thread>
#include <chrono>

#include "core/tunnel/connection.hpp"
#include "core/common/opt.hpp"
#include "core/tunnel/socket_management.hpp"
#include "core/common/shared.hpp"
#include "core/database/database.hpp"
#include "core/common/signal_handler.hpp"
#include "core/database/client.hpp"
#include "core/common/input.hpp"
#include "core/database/authentication.hpp"
#include "core/common/console.hpp"
#include "core/children/api.hpp"

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
  std::thread api_thread(api_control_thread_func);

  while (!shared_resources::global_flag_kill && !shared_resources::flag_api_kill && !shared_resources::flag_api_service_running) std::this_thread::yield();
  shared_resources::api_stream = popen("./sphere-linked-server-api", "w");
  if (!shared_resources::api_stream) {
    console(ERROR, API_START_PROCESS_FAILED, std::to_string(errno).c_str(), "main");
    shared_resources::flag_api_kill = true;
  }

  ssl_control_thread.join();
  update_client_db_thread.join();
  input_thread.join();
  api_thread.join();

  if (!shared_resources::flag_handling_signal) signal_handler(0);
  return 0;
}