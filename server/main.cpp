//
// Created by Jhean Lee on 2024/10/2.
//

#include <thread>
#include <csignal>

#include <unistd.h>
#if !(defined(__clang__) && defined(__APPLE__))
  #include <sys/syslog.h>
#endif

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
  #if !(defined(__clang__) && defined(__APPLE__))
    openlog("aqueduct-server", LOG_PID | LOG_CONS, LOG_USER);
  #endif
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
  pid_t pid_api = fork();
  if (pid_api < 0) {
    console(ERROR, API_START_PROCESS_FAILED, std::to_string(errno).c_str(), "main");
  } else if (pid_api == 0) {
    //  api child

    const char *args[7];
    args[0] = "./aqueduct-server-api";
    args[1] = "--database";
    args[2] = db_path;
    args[3] = "--verbose";
    args[4] = std::to_string(verbose_level).c_str();
    args[5] = (shared_resources::daemon_mode) ? "--daemon-mode" : nullptr;
    args[6] = nullptr;

    execvp("./aqueduct-server-api", const_cast<char *const *> (args));

    //  api child call failure
    console(ERROR, API_START_PROCESS_FAILED, std::to_string(errno).c_str(), "main_api_child");
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }

  shared_resources::pid_api = pid_api;

  ssl_control_thread.join();
  update_client_db_thread.join();
  input_thread.join();
  api_thread.join();

  if (!shared_resources::flag_handling_signal) signal_handler(0);
  return 0;
}