//
// Created by Jhean Lee on 2024/10/2.
//
#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <cstring>

#include <unistd.h>
#include <uuid/uuid.h>

#include "connection.hpp"
#include "config.hpp"
#include "message.hpp"
#include "shared.hpp"

void heartbeat_thread_func(int &client_fd, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill) {
  char outbuffer[1024] = {0};
  Message heartbeat_message = {.type = HEARTBEAT, .string = ""};

  std::chrono::system_clock::time_point timer;
  std::chrono::seconds heartbeat_duration;

  while (!flag_kill) {
    try {
      send_message(client_fd, outbuffer, sizeof(outbuffer), heartbeat_message);
    } catch (int err) {
      std::cerr << "[Warninng] Unable to send message (connection::heartbeat)\n";
    }

    //  start timing
    timer = std::chrono::system_clock::now();
    flag_heartbeat_received = false;

    //  wait for heartbeat
    while (!flag_heartbeat_received) {
      heartbeat_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);

      if (heartbeat_duration > std::chrono::seconds(heartbeat_timeout_sec)) {
        std::cout << "[Info] Heartbeat timeout: " << inet_ntoa(client_addr.sin_addr) << ':' << (int) ntohs(client_addr.sin_port) << '\n';
        flag_kill = true; return;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
  }
}

void session_thread_func(int client_fd, sockaddr_in client_addr, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id) {
  bool flag_first_msg = false, flag_connect_type = false, flag_proxy_type = false;
  std::atomic<bool> flag_heartbeat_received(false), flag_kill(false);
  int recv_status = 0;
  char inbuffer[1024] = {0};
  Message message {.type = -1, .string = ""};

  fd_set read_fds;
  timeval timev = {.tv_sec = 0, .tv_usec = 50};

  std::thread heartbeat_thread;
  std::thread proxy_service_port_thread; // user-proxyserver connection, stream to service(client)
  std::thread proxy_thread;

  std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
  std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

  while (!flag_kill && (flag_first_msg || duration <= std::chrono::seconds(first_message_timeout_sec))) {
    if (!flag_first_msg) duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

    recv_status = read_message_non_block(read_fds, client_fd, timev, inbuffer, sizeof(inbuffer), message);
    if (recv_status < 0) {
      flag_kill = true;
    } else if (recv_status > 0){
      switch (message.type) {
        case CONNECT:
          flag_first_msg = true; flag_connect_type = true;
          heartbeat_thread = std::thread(heartbeat_thread_func, std::ref(client_fd), std::ref(client_addr), std::ref(flag_heartbeat_received), std::ref(flag_kill));
          proxy_service_port_thread = std::thread(proxy_service_port_thread_func, std::ref(flag_kill), std::ref(external_user_id), std::ref(client_fd), std::ref(client_addr));
          break;
        case HEARTBEAT:
          flag_heartbeat_received = true;
          break;
        case REDIRECT:
          flag_first_msg = true;

          if (external_user_id.find(message.string) != external_user_id.end()) {
            std::unique_lock<std::mutex> lock(shared_resources::external_user_mutex);
            std::pair<int, sockaddr_in> external_user = external_user_id[message.string];
            external_user_id.erase(message.string);
            lock.unlock();

            proxy_thread = std::thread(proxy_thread_func, client_fd, external_user.first, std::ref(flag_kill));
            flag_proxy_type = true;
          } else flag_kill = true;
          goto proxy;
      }
    }
  }
  flag_kill = true;
  proxy:
  if (flag_connect_type) { heartbeat_thread.join(); proxy_service_port_thread.join(); }
  if (flag_proxy_type) { proxy_thread.join(); }

  close(client_fd);
  std::cout << "[Info] Connection closed: " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << '\n';
}

void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id, int &client_fd, sockaddr_in &client_addr) {
  int server_proxy_fd = 0, status = 0, on = 1, proxy_port = 0, external_user_fd = 0;
  char outbuffer[1024] = {0}, uuid_str[37] = {0};

  struct sockaddr_in server_proxy_addr = {.sin_family = AF_INET}, external_user_addr = {.sin_family = AF_INET};
  inet_pton(AF_INET, host, &server_proxy_addr.sin_addr);

  uuid_t uuid;
  Message message{.type = STREAM_PORT, .string = ""};

  //  create socket
  server_proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_proxy_fd == -1) { std::cerr << "[Error] Failed to create socket (connection::proxy_service)\n"; exit(1); }

  //  set proxy port
  std::unique_lock<std::mutex> ports_assign_lock(shared_resources::ports_mutex);
  if (proxy_ports_available.empty()) { std::cerr << "[Warning] No ports available (connection::proxy_service)\n"; flag_kill = true; return; }
  for (int i = 0; i < proxy_ports_available.size(); i++) {
    server_proxy_addr.sin_port = htons(proxy_ports_available.front());

    status = bind(server_proxy_fd, (struct sockaddr *) &server_proxy_addr, sizeof(server_proxy_addr));
    if (status == -1) continue;

    status = listen(server_proxy_fd, connection_limit);
    if (status == -1) continue;
    if (setsockopt(server_proxy_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) continue;

    std::cout << "[Info] Opened proxy port at: " << proxy_ports_available.front() << '\n';
    proxy_port = proxy_ports_available.front();
    proxy_ports_available.pop();
    break;
  }
  ports_assign_lock.unlock();

  message.string = std::to_string((int) ntohs(server_proxy_addr.sin_port));
  send_message(client_fd, outbuffer, sizeof(outbuffer), message);

  //  accepting external connections
  while (!flag_kill) {
    socklen_t external_user_addrlen = sizeof(external_user_addr);
    external_user_fd = accept(server_proxy_fd, (struct sockaddr *) &external_user_addr, &external_user_addrlen);
    std::cout << "[Info] Accepted external connection: " << inet_ntoa(external_user_addr.sin_addr) << ':' << (int) ntohs(external_user_addr.sin_port) << '\n';

    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);

    message.type = REDIRECT;
    message.string = std::string(uuid_str);
    std::unique_lock<std::mutex> user_id_lock(shared_resources::external_user_mutex);
    external_user_id.try_emplace(message.string, std::pair(external_user_fd, external_user_addr));
    user_id_lock.unlock();

    send_message(client_fd, outbuffer, sizeof(outbuffer), message);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  close(server_proxy_fd);
  std::unique_lock<std::mutex> ports_close_lock(shared_resources::ports_mutex);
  proxy_ports_available.push(proxy_port);
  ports_close_lock.unlock();
}

void proxy_thread_func(int service_fd, int external_user_fd, std::atomic<bool> &flag_kill) {
  std::cout << "[Info] Proxying started\n";

  int ready_for_call = 0;
  ssize_t nbytes = 0;
  char buffer[32768];

  fd_set read_fd;
  timeval timev = {.tv_sec = 0, .tv_usec = 0};

  while (!flag_kill) {
    /// service -> external_user
    FD_ZERO(&read_fd); FD_SET(service_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;

    ready_for_call = select(service_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select (connection::proxy_thread)\n"; break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(service_fd, buffer, sizeof(buffer), 0);

      if (nbytes <= 0) { std::cout << "[Info] Service has closed connection\n"; break; }
      if (send(external_user_fd, buffer, nbytes, 0) < 0) { std::cerr << "[Warning] Unable to send buffer to user (connection:proxy_thread)\n"; break; }
    }

    /// external_user -> service
    FD_ZERO(&read_fd); FD_SET(external_user_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;

    ready_for_call = select(external_user_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select (connection::proxy_thread)\n";
      break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(external_user_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) { std::cout << "[Info] External user has closed connection\n"; break; }
      if (send(service_fd, buffer, nbytes, 0) < 0) { std::cerr << "[Warning] Error occurred while sending buffer to service (connection:proxy_thread)\n"; }
    }
  }

  close(service_fd); close(external_user_fd);
  flag_kill = true;
  std::cout << "[Info] Proxying ended\n";
}