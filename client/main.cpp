//
// Created by Jhean Lee on 2024/10/2.
//

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>

#include <unistd.h>

#include "message.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "opt.hpp"
#include "socket_management.hpp"

int main(int argc, char *argv[]) {
  opt_handler(argc, argv);
  init_openssl();

  bool flag_service_thread = false, flag_server_active = false;
  int server_fd = 0, status = 0;
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  std::atomic<bool> flag_kill(false);
  std::queue<std::string> user_id;
  std::chrono::system_clock::time_point timer;
  std::chrono::seconds server_response_duration;

  std::thread service_thread;

  Message message{.type = CONNECT, .string = ""};
  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  fd_set readfd;
  timeval timev = {.tv_sec = select_timeout_session_sec, .tv_usec = select_timeout_session_millisec};

  //  create, connect socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) { std::cerr << "[Error] Failed to create socket \033[2;90m(main)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }
  if (connect(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) { std::cerr << "[Error] Failed to connect to host \033[2;90m(main)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

  //  ssl context, turn plain socket into ssl connection
  SSL_CTX *ctx = create_context();
  SSL *server_ssl = SSL_new(ctx);
  SSL_set_fd(server_ssl, server_fd);
  if (SSL_connect(server_ssl) <= 0) { std::cerr << "[Error] Failed to SSL_connect \033[2;90m(main)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

  std::cout << "[Info] Connected to " << host << ':' << host_main_port << " \033[2;90m(main)\033[0m\n";

  //  send CONNECT message
  if (ssl_send_message(server_ssl, outbuffer, sizeof(outbuffer), message) <= 0){
    std::cerr << "[Warning] Failed to send message \033[2;90m(main)\033[0m\n";
  }
  timer = std::chrono::system_clock::now();

  while (!flag_kill) {
    if (!flag_server_active) {  //  ensure server is alive
      server_response_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);
      if (server_response_duration > std::chrono::seconds(60)) {
        std::cerr << "[Error] Host response timed out \033[2;90m(main)\033[0m\n"; flag_kill = true; break;
      }
    }

    //  reading message without blocking
    FD_ZERO(&readfd);
    FD_SET(server_fd, &readfd);
    timev = {.tv_sec = select_timeout_session_sec, .tv_usec = select_timeout_session_millisec};
    status = select(server_fd + 1, &readfd, nullptr, nullptr, &timev);
    int nbytes;
    try {
      if (status < 0) {
        std::cerr << "[Error] Invalid file descriptor passed to select \033[2;90m(main)\033[0m\n"; flag_kill = true; break;
      } else if (status > 0) {
        nbytes = ssl_recv_message(server_ssl, inbuffer, sizeof(inbuffer), message);
      } else {
        continue;
      }
    } catch (int err) {
      std::cerr << "[Warning] Failed to receive message \033[2;90m(main)\033[0m\n";
    }

    if (nbytes <= 0) {
      SSL_shutdown(server_ssl);
      SSL_free(server_ssl);
      SSL_CTX_free(ctx);
      close(server_fd);
      std::cout << "[Info] Connection to host closed \033[2;90m(main)\033[0m\n";
      flag_kill = true; break;
    } else {
      flag_server_active = true;

      switch (message.type) {
        case HEARTBEAT:
          send_heartbeat_message(server_ssl, outbuffer);
          break;
        case STREAM_PORT:
          std::cout << "[Info] Started streaming to " << readable_host << ':' << message.string << " \033[2;90m(main)\033[0m\n";
          service_thread = std::thread(service_thread_func, std::ref(flag_kill), std::ref(user_id));
          flag_service_thread = true;
          break;
        case REDIRECT:
          user_id.push(message.string);
          break;
        case AUTHENTICATION:
          send_auth_message(server_ssl, outbuffer, sizeof(outbuffer), message.string);
          std::cout << "[Info] Authentication request sent \033[2;90m(main)\033[0m\n";
          break;
        case AUTH_SUCCESS:
          std::cout << "[Info] Authentication success \033[2;90m(main)\033[0m\n";
          break;
        case AUTH_FAILED:
          flag_kill = true;
          std::cerr << "[Error] Authentication failed \033[2;90m(main)\033[0m\n";
          break;
        case DB_ERROR:
          flag_kill = true;
          std::cerr << "[Error] Server encountered an error with database \033[2;90m(main)\033[0m\n";
          break;
      }

      message = {.type = -1, .string = ""};
    }
  }

  if (flag_service_thread) service_thread.join();
  cleanup_openssl();
  return 0;
}
