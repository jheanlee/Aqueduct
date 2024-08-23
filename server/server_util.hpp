#ifndef TUNNEL_SERVER_UTIL_HPP
  #define TUNNEL_SERVER_UTIL_HPP

  #include <iostream>
  #include <thread>
  #include <chrono>
  #include <atomic>
  #include <string>
  #include <unordered_map>

  #include <unistd.h>
  #include <arpa/inet.h>

  #include "../common/message.hpp"
  #include "server_config.hpp"
  #include "../common/random_key.hpp"


void heartbeat(int &client_fd, sockaddr_in &client_addr,std::atomic<bool> &echo_heartbeat, std::atomic<bool> &close_session_flag);

void session(int client_fd, sockaddr_in client_addr);

void stream_port(int &client_fd, int &new_port, std::atomic<bool> &port_connected, std::atomic<bool> &close_session_flag, std::unordered_map<int, int> &user_socket_fds);

void proxy(int user_fd);

#endif