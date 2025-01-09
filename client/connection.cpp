//
// Created by Jhean Lee on 2024/10/30.
//

#include <thread>
#include <vector>
#include <iostream>
#include <cstring>

#include <unistd.h>

#include "connection.hpp"
#include "message.hpp"
#include "config.hpp"
#include "socket_management.hpp"

void send_heartbeat_message(SSL *server_ssl, char *buffer) {
  Message message{.type = HEARTBEAT, .string = ""};
  ssl_send_message(server_ssl, buffer, sizeof(buffer), message);
}

void service_thread_func(std::atomic<bool> &flag_kill, std::queue<std::string> &user_id) {
  std::vector<std::thread> proxy_threads;

  // ##service##
  int service_fd = 0;
  struct sockaddr_in service_addr{.sin_family = AF_INET, .sin_port = htons(local_service_port)};
  inet_pton(AF_INET, local_service, &service_addr.sin_addr);

  while (!flag_kill) {
    while (!flag_kill && user_id.empty()) std::this_thread::yield();

    //  create socket (service)
    service_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (service_fd == -1) { std::cerr << "[Error] Failed to create socket for service (connection)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }
    
    //  connect (service)
    if (connect(service_fd, (struct sockaddr *) &service_addr, sizeof(service_addr))) { std::cerr << "[Error] Unable to connect to service (connection)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    std::cout << "[Info] Connected to service\n";

    // ##host##
    int host_fd = 0;
    char buffer[1024] = {0};

    Message redirect_message{.type = REDIRECT, .string = user_id.front()};
    user_id.pop();

    struct sockaddr_in host_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
    inet_pton(AF_INET, host, &host_addr.sin_addr);

    //  create, connect socket (host)
    host_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (host_fd == -1) { std::cerr << "[Error] Failed to create socket for host (connection)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }
    if (connect(host_fd, (struct sockaddr *) &host_addr, sizeof(host_addr))) { std::cerr << "[Error] Unable to connect to host (connection)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    //  ssl context, turn plain socket into ssl connection
    SSL_CTX *ctx = create_context();
    SSL *host_ssl = SSL_new(ctx);
    SSL_set_fd(host_ssl, host_fd);
    if (SSL_connect(host_ssl) <= 0) { std::cerr << "[Error] Unable to SSL_connect (connection)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    std::cout << "[Info] Connected to host for redirect id: "<< redirect_message.string << '\n';

    ssl_send_message(host_ssl, buffer, sizeof(buffer), redirect_message);
    proxy_threads.emplace_back(proxy_thread_func, std::ref(flag_kill), host_ssl, host_fd, host_addr, service_fd);
  }

  for (std::thread &t : proxy_threads) t.join();
  close(service_fd);
}

void proxy_thread_func(std::atomic<bool> &flag_kill, SSL *host_ssl, int host_fd, sockaddr_in host_addr, int service_fd) {
  std::cout << "[Info] Proxying started\n";

  int ready_for_call = 0;
  ssize_t nbytes = 0;
  char buffer[32768];
  fd_set read_fd;
  timeval timev = {.tv_sec = select_timeout_proxy_sec, .tv_usec = select_timeout_proxy_millisec};

  while (!flag_kill) {
    // service -> host
    FD_ZERO(&read_fd); FD_SET(service_fd, &read_fd);
    timev = {.tv_sec = select_timeout_proxy_sec, .tv_usec = select_timeout_proxy_millisec};

    ready_for_call = select(service_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select (connection::proxy_thread)\n"; break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(service_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) { std::cout << "[Info] Service has closed connection\n"; break; }
      if (SSL_write(host_ssl, buffer, nbytes) < 0) { std::cerr << "[Warning] Unable to send buffer to user (connection:proxy_thread)\n"; break; }
    }

    // host -> service
    FD_ZERO(&read_fd); FD_SET(host_fd, &read_fd);
    timev = {.tv_sec = select_timeout_proxy_sec, .tv_usec = select_timeout_proxy_millisec};

    ready_for_call = select(SSL_get_fd(host_ssl) + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = SSL_read(host_ssl, buffer, sizeof(buffer));
      if (nbytes <= 0) { std::cout << "[Info] External user has closed connection\n"; break; }
      if (send(service_fd, buffer, nbytes, 0) < 0) { std::cerr << "[Warning] Error occurred while sending buffer to service (connection:proxy_thread)\n"; }
    }
  }
  SSL_shutdown(host_ssl);
  SSL_free(host_ssl);
  close(service_fd); close(host_fd);
  std::cout << "[Info] Proxying ended\n";
}