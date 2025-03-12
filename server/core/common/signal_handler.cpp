//
// Created by Jhean Lee on 2025/1/24.
//

#include <csignal>

#include <sys/wait.h>
#include <sqlite3.h>
#if !(defined(__clang__) && defined(__APPLE__))
  #include <sys/syslog.h>
#endif

#include "signal_handler.hpp"
#include "console.hpp"
#include "shared.hpp"
#include "../tunnel/socket_management.hpp"

void register_signal() {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGFPE, signal_handler);
  signal(SIGILL, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGPIPE, SIG_IGN);
}

void signal_handler(int signal) {
  shared_resources::flag_handling_signal = true;
  shared_resources::global_flag_kill = true;
  console(NOTICE, SIGNAL, std::to_string(signal).c_str(), "signal_handler");

  //  close api child
  if (shared_resources::pid_api != 0) {
    kill(shared_resources::pid_api, SIGTERM);
    int api_exit_status;
    waitpid(shared_resources::pid_api, &api_exit_status, 0);
    console(NOTICE, API_PROCESS_ENDED, (std::to_string(api_exit_status) + ' ').c_str(), "signal_handler");
  }

  //  close db
  if (shared_resources::db) {
    sqlite3_stmt *stmt;
    while ((stmt = sqlite3_next_stmt(shared_resources::db, nullptr)) != nullptr) {
      sqlite3_finalize(stmt);
    }

    if (sqlite3_get_autocommit(shared_resources::db) == 0) {
      sqlite3_exec(shared_resources::db, "ROLLBACK", nullptr, nullptr, nullptr);
    }

    if (sqlite3_close(shared_resources::db) == SQLITE_OK) {
      console(NOTICE, SQLITE_CLOSE_SUCCESS, nullptr, "signal_handler");
    } else {
      console(ERROR, SQLITE_CLOSE_FAILED, sqlite3_errmsg(shared_resources::db), "signal_handler");
    }
  }

  cleanup_openssl();
  #if !(defined(__clang__) && defined(__APPLE__))
    closelog();
  #endif
  exit(signal);
}
