//
// Created by Jhean Lee on 2025/2/5.
//

#include <csignal>
#include <cstdlib>

#include "signal_handler.hpp"
#include "../tunnel/socket_management.hpp"
#include "console.hpp"

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
  cleanup_openssl();
  console(NOTICE, SIGNAL, std::to_string(signal).c_str(), "signal_handler");
  exit(signal);
}