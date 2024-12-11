//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_CONNECTION_HPP
  #define TUNNEL_CONNECTION_HPP

  #include <atomic>
  #include <unordered_map>

  #include <arpa/inet.h>

  void heartbeat_thread_func(int &client_fd, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill);
  void session_thread_func(int client_fd, sockaddr_in client_addr, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id);
  void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id, int &client_fd, sockaddr_in &client_addr);
  void proxy_thread_func(int service_fd, int external_user_fd, std::atomic<bool> &flag_kill);

#endif //TUNNEL_CONNECTION_HPP