//
// Created by Jhean Lee on 2024/10/30.
//

#ifndef TUNNEL_CONNECTION_HPP
  #define TUNNEL_CONNECTION_HPP

  #include <thread>
  #include <atomic>
  #include <queue>
  #include <vector>

  #include <unistd.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>

  #include "message.hpp"
  #include "config.hpp"
  #include "thread_safe_queue.hpp"

  void send_heartbeat_message(int &socket_fd, char *buffer);
  void service_thread_func(std::atomic<bool> &flag_kill, ThreadSafeQueue<std::string> &user_id);
  void proxy_thread_func(std::atomic<bool> &flag_kill, int host_fd, sockaddr_in host_addr, int service_fd);


#endif //TUNNEL_CONNECTION_HPP