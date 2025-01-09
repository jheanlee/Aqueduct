//
// Created by Jhean Lee on 2024/10/2.
//

#include <atomic>
#include <thread>

#include "config.hpp"
#include "connection.hpp"
#include "opt.hpp"
#include "socket_management.hpp"


int main(int argc, char *argv[]) {
  opt_handler(argc, argv);
  init_proxy_ports_available();
  init_openssl();

  std::atomic<bool> flag_kill = false;
  std::thread control_thread;
  std::thread ssl_control_thread;

  ssl_control_thread = std::thread(ssl_control_thread_func, std::ref(flag_kill));

  ssl_control_thread.join();
  return 0;
}