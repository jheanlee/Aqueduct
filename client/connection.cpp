//
// Created by Jhean Lee on 2024/10/30.
//

#include "connection.hpp"


void send_heartbeat_message(int &socket_fd, char *buffer) {
  Message message{.type = HEARTBEAT, .string = ""};

  send_message(socket_fd, buffer, sizeof(buffer), message);
//  std::cout << "Sent: " << message.type << ", " << message.string << '\n';
}

void service_thread_func(std::atomic<bool> &flag_kill, ThreadSafeQueue<std::string> &user_id) {
  std::vector<std::thread> proxy_threads;

  /// service
  int service_fd = 0, service_status = 0;
  struct sockaddr_in service_addr{.sin_family = AF_INET, .sin_port = htons(local_service_port)};
  inet_pton(AF_INET, local_service, &service_addr.sin_addr);

  while (!flag_kill) {
    while (!flag_kill && user_id.empty()) std::this_thread::yield();

    //  create socket
    service_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (service_fd == -1) { std::cerr << "[Error] Failed to create socket for service (connection)\n"; exit(1); }
    
    //  connect
    service_status = connect(service_fd, (struct sockaddr *) &service_addr, sizeof(service_addr));
    if (service_status == -1) { std::cerr << "[Error] Unable to connect to service (connection)\n"; exit(1); }

    std::cout << "[Info] Connected to service\n";

    /// host
    int host_fd = 0, host_status = 0;
    Message redirect_message{.type = REDIRECT, .string = user_id.front()};
    user_id.pop();
    char buffer[1024] = {0};

    struct sockaddr_in host_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
    inet_pton(AF_INET, host, &host_addr.sin_addr);

    //  create socket
    host_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (host_fd == -1) {
      std::cerr << "[Error] Failed to create socket for host (connection)\n";
      exit(1);
    }

    //  connect
    host_status = connect(host_fd, (struct sockaddr *) &host_addr, sizeof(host_addr));
    if (host_status == -1) {
      std::cerr << "[Error] Unable to connect to host (connection)\n";
      exit(1);
    }

    std::cout << "[Info] Connected to host for redirect id: "<< redirect_message.string << '\n';
    send_message(host_fd, buffer, sizeof(buffer), redirect_message);

    proxy_threads.emplace_back(proxy_thread_func, std::ref(flag_kill), host_fd, host_addr, service_fd);
  }

  for (std::thread &t : proxy_threads) t.join();
  close(service_fd);
}

void proxy_thread_func(std::atomic<bool> &flag_kill, int host_fd, sockaddr_in host_addr, int service_fd) {
  std::cout << "[Info] Proxying started\n";
  fd_set read_fd;
  timeval timev = {.tv_sec = 0, .tv_usec = 0};
  int ready_for_call = 0, nbytes = 0;
  char buffer[32768];

  while (!flag_kill) {
    FD_ZERO(&read_fd);
    FD_SET(service_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;
    ready_for_call = select(service_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select (connection::proxy_thread)\n";
      break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(service_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) {
        std::cout << "[Info] Service has closed connection\n";
        break;
      }
      if (send(host_fd, buffer, nbytes, 0) < 0) {
        std::cerr << "[Warning] Unable to send buffer to user (connection:proxy_thread)\n";
        break;
      }
    }

    FD_ZERO(&read_fd);
    FD_SET(host_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;
    ready_for_call = select(host_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select (connection::proxy_thread)\n";
      break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(host_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) {
        std::cout << "[Info] External user has closed connection\n";
        break;
      }
      if (send(service_fd, buffer, nbytes, 0) < 0) {
        std::cerr << "[Warning] Error occurred while sending buffer to service (connection:proxy_thread)\n";
      }
    }
  }

  close(service_fd); close(host_fd);
  std::cout << "[Info] Proxying ended\n";
}