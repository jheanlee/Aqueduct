//
// Created by Jhean Lee on 2025/3/4.
//

#ifndef AQUEDUCT_API_HPP
  #define AQUEDUCT_API_HPP

  #include <cstdio>
  #include <mutex>
  #include <atomic>

  #include <sys/un.h>

  void api_control_thread_func();
  void api_session_thread_func(int api_fd, sockaddr_un api_addr);
  void api_heartbeat_thread_func(std::atomic<bool> &flag_kill, int &api_fd, std::mutex &send_mutex, std::atomic<bool> &flag_heartbeat_received);

#endif //AQUEDUCT_API_HPP
