//
// Created by Jhean Lee on 2024/10/2.
//

#include "connection.hpp"

void heartbeat_thread_func(int &client_fd, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill) {
  char outbuffer[1024] = {0};
  Message heartbeat_message = {.type = HEARTBEAT, .string = ""};

  std::chrono::system_clock::time_point time_point;
  std::chrono::seconds duration;

  while (!flag_kill) {
    try {
      send_message(client_fd, outbuffer, sizeof(outbuffer), heartbeat_message);
      std::cout << "To " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " " \
        << "Sent: " << heartbeat_message.type << ", " << heartbeat_message.string << '\n';
    } catch (int err) {
      std::cerr << "[connection.cpp] Error occurred sending message. \n";
    }

    //  start timing
    time_point = std::chrono::system_clock::now();
    flag_heartbeat_received = false;

    //  wait for heartbeat
    while (!flag_heartbeat_received) {
      duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

      if (duration > std::chrono::seconds(heartbeat_timeout_sec)) {
        std::cout << inet_ntoa(client_addr.sin_addr) << ':' << (int) ntohs(client_addr.sin_port) << ' ' \
          << "Heartbeat timeout. \n";
        flag_kill = true;
        return;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
  }
}

void session_thread_func(int client_fd, sockaddr_in client_addr, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id) {
  std::atomic<bool> flag_heartbeat_received (false);
  std::atomic<bool> flag_kill (false);
  bool flag_first_msg = false;
  bool flag_connect_type = false, flag_proxy_type = false;

  fd_set read_fds;
  timeval timev = {.tv_sec = 0, .tv_usec = 50};
  int recv_status = 0;

  std::thread heartbeat_thread;
  std::thread proxy_service_port_thread; // user-proxyserver connection, stream to service(client)
  std::thread proxy_thread;

  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message {.type = -1, .string = ""};

  std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
  std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

  while (!flag_kill && (flag_first_msg || duration <= std::chrono::seconds(first_message_timeout_sec))) {
    if (!flag_first_msg) duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

    recv_status = read_message_non_block(read_fds, client_fd, timev, inbuffer, sizeof(inbuffer), message);
    if (recv_status < 0) {
      flag_kill = true;
    } else if (recv_status > 0){

      std::cout << "From "  << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " "     \
        << "Recv: " << message.type << ", " << message.string << '\n';
      switch (message.type) {
        case CONNECT:
          flag_first_msg = true;
          flag_connect_type = true;
          heartbeat_thread = std::thread(heartbeat_thread_func, std::ref(client_fd), std::ref(client_addr), std::ref(flag_heartbeat_received), std::ref(flag_kill));
          proxy_service_port_thread = std::thread(proxy_service_port_thread_func, std::ref(flag_kill), std::ref(external_user_id), std::ref(client_fd), std::ref(client_addr));
          break;
        case HEARTBEAT:
          flag_heartbeat_received = true;
          break;
        case REDIRECT:
          flag_first_msg = true;

          if (external_user_id.find(message.string) != external_user_id.end()) {
            std::pair<int, sockaddr_in> p = external_user_id[message.string];
            external_user_id.erase(message.string);
            proxy_thread = std::thread(proxy_thread_func, client_fd, p.first, std::ref(flag_kill));
            flag_proxy_type = true;
          } else flag_kill = true;

          break;
      }
    }
  }
  flag_kill = true;

  if (flag_connect_type) { heartbeat_thread.join(); proxy_service_port_thread.join(); }
  if (flag_proxy_type) { proxy_thread.join(); }

  close(client_fd);
  std::cout << "Connection with " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " closed.\n";
}

void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id, int &client_fd, sockaddr_in &client_addr) {
  int server_proxy_fd = 0, status = 0, on = 1, proxy_port = 0;
  int external_user_fd = 0;
  struct sockaddr_in server_proxy_addr = {.sin_family = AF_INET}, external_user_addr = {.sin_family = AF_INET};
  inet_pton(AF_INET, host, &server_proxy_addr.sin_addr);

  char outbuffer[1024] = {0}, uuid_str[37] = {0};
  uuid_t uuid;
  Message message{.type = STREAM_PORT, .string = ""};

  //  create socket
  server_proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_proxy_fd == -1) { std::cerr << "[connection.cpp] Failed to create socket. \n"; exit(1); }

  //  set proxy port
  if (proxy_port_available.empty()) { std::cerr << "[connection.cpp] no ports available \n"; flag_kill = true; return; }
  for (int i = 0; i < proxy_port_available.size(); i++) {
    server_proxy_addr.sin_port = htons(proxy_port_available[i]);

    status = bind(server_proxy_fd, (struct sockaddr *) &server_proxy_addr, sizeof(server_proxy_addr));
    if (status == -1) continue;

    status = listen(server_proxy_fd, connection_limit);
    if (status == -1) continue;
    if (setsockopt(server_proxy_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) continue;

    std::cout << "Opened proxy port at: " << proxy_port_available[i] << '\n';
    proxy_port = proxy_port_available[i];
    proxy_port_available.erase(proxy_port_available.begin() + i);
    break;
  }

  message.string = std::to_string((int) ntohs(server_proxy_addr.sin_port));
  send_message(client_fd, outbuffer, sizeof(outbuffer), message);

  while (!flag_kill) {
    socklen_t external_user_addrlen = sizeof(external_user_addr);
    external_user_fd = accept(server_proxy_fd, (struct sockaddr *) &external_user_addr, &external_user_addrlen);
    std::cout << "Accepted connection from " << inet_ntoa(external_user_addr.sin_addr) << ':' << (int) ntohs(external_user_addr.sin_port) << '\n';
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    message.type = REDIRECT;
    message.string = std::string(uuid_str);
    external_user_id.try_emplace(message.string, std::pair(external_user_fd, external_user_addr));

    std::cout << "To " << inet_ntoa(client_addr.sin_addr) << ':' << (int) ntohs(client_addr.sin_port) << ' ' \
      << "Sent: " << message.type << ", " << message.string << '\n';

    send_message(client_fd, outbuffer, sizeof(outbuffer), message);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));  //  period can be adjusted
  }

  close(server_proxy_fd);
  proxy_port_available.push_back(proxy_port);
}

void proxy_thread_func(int service_fd, int user_fd, std::atomic<bool> &flag_kill) {
  std::cout << "Proxying started\n";
  fd_set read_fd;
  timeval timev = {.tv_sec = 0, .tv_usec = 0};
  int ready_for_call = 0, nbytes = 0;
  char buffer[1024] = {0};

  while (!flag_kill) {
    FD_ZERO(&read_fd);
    FD_SET(service_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;
    ready_for_call = select(service_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[connection.cpp] Error occurred in select(). \n";
      close(service_fd); close(user_fd);
      flag_kill = true;
      break;
    } else if (ready_for_call > 0) {
      nbytes = recv(service_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) {
        close(service_fd); close(user_fd);
        flag_kill = true;
        break;
      }
      send(user_fd, buffer, strlen(buffer), 0);
    }

    FD_ZERO(&read_fd);
    FD_SET(user_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;
    ready_for_call = select(user_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[connection.cpp] Error occurred in select(). \n";
      close(service_fd); close(user_fd);
      flag_kill = true;
      break;
    } else if (ready_for_call > 0) {
      nbytes = recv(user_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) {
        close(service_fd); close(user_fd);
        flag_kill = true;
        break;
      }
      send(service_fd, buffer, strlen(buffer), 0);
    }
  }
  close(user_fd);
  close(service_fd);
}