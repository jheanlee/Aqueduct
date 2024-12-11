//
// Created by Jhean Lee on 2024/10/30.
//

#ifndef TUNNEL_CONNECTION_HPP
  #define TUNNEL_CONNECTION_HPP

  #include <atomic>
  #include <queue>
  #include <string>

  #include <sys/socket.h>
  #include <arpa/inet.h>


  void send_heartbeat_message(int &socket_fd, char *buffer);
  void service_thread_func(std::atomic<bool> &flag_kill, std::queue<std::string> &user_id);
  void proxy_thread_func(std::atomic<bool> &flag_kill, int host_fd, sockaddr_in host_addr, int service_fd);


#endif //TUNNEL_CONNECTION_HPP