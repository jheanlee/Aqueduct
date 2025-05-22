//
// Created by Jhean Lee on 2025/2/5.
//

#include <csignal>
#include <cstdlib>

#if !(defined(__clang__) && defined(__APPLE__))
  #include <sys/syslog.h>
#endif

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
  if (signal == 0) {
    console(NOTICE, SIGNAL, std::to_string(signal).c_str(), "signal_handler");
  } else {
    console(WARNING, SIGNAL, std::to_string(signal).c_str(), "signal_handler");
  }

  #if !(defined(__clang__) && defined(__APPLE__))
    closelog();
  #endif

  exit(signal);
}