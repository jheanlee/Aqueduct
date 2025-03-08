//
// Created by Jhean Lee on 2025/2/5.
//

#include <csignal>
#include <cstdlib>

#include "signal_handler.hpp"
#include "../tunnel/socket_management.hpp"

void register_signal() {
  signal(SIGPIPE, SIG_IGN);
}

void signal_handler(int signal) {
  cleanup_openssl();

  exit(signal);
}