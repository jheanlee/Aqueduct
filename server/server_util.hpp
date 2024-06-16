#ifndef TUNNEL_SERVER_UTIL_HPP
  #define TUNNEL_SERVER_UTIL_HPP

  #include <iostream>
  #include <thread>
  #include <chrono>
  #include <atomic>

  #include <unistd.h>
  #include <arpa/inet.h>

  #include "../common/message.hpp"
  #include "../common/config.hpp"


void heartbeat(int &client_fd, sockaddr_in &client_addr,std::atomic<bool> &echo_heartbeat, std::atomic<bool> &close_session_flag);

void session(int client_fd, sockaddr_in client_addr);

void listen_new_port();

#endif