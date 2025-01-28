//
// Created by Jhean Lee on 2025/1/24.
//
#include <sqlite3.h>
#include <csignal>

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
  signal(SIGPIPE, signal_handler);
}

void signal_handler(int signal) {
  console(WARNING, SIGNAL, std::to_string(signal).c_str(), "signal_handler");

  shared_resources::flag_handling_signal = true;
  shared_resources::global_flag_kill = true;
  if (shared_resources::db != nullptr) {
    sqlite3_stmt *stmt;
    while ((stmt = sqlite3_next_stmt(shared_resources::db, nullptr)) != nullptr) {
      sqlite3_finalize(stmt);
    }

    if (sqlite3_get_autocommit(shared_resources::db) == 0) {
      sqlite3_exec(shared_resources::db, "ROLLBACK", nullptr, nullptr, nullptr);
    }

    if (sqlite3_close(shared_resources::db) == SQLITE_OK) {
      console(INFO, SQLITE_CLOSE_SUCCESS, nullptr, "signal_handler");
    } else {
      console(ERROR, SQLITE_CLOSE_FAILED, sqlite3_errmsg(shared_resources::db), "signal_handler");
    }
  }

  cleanup_openssl();
  exit(signal);
}
