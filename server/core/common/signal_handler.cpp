//
// Created by Jhean Lee on 2025/1/24.
//
#include <sqlite3.h>
#include <csignal>
#include <cstdio>

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
  console(WARNING, SIGNAL, std::to_string(signal).c_str(), "signal_handler");

  //  close api child
  if (shared_resources::api_stream) {
    fputs("k", shared_resources::api_stream); //  kill signal TODO (api rust)

    int api_exit_status = pclose(shared_resources::api_stream);
    if (api_exit_status < 0) {
      console(ERROR, API_PCLOSE_FAILED, (std::to_string(errno) + ' ').c_str(), "signal_handler");
    } else {
      console(INFO, API_PCLOSE_SUCCESS, (std::to_string(api_exit_status) + ' ').c_str(), "signal_handler");
    }
  }

  //  close db
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
