//
// Created by Jhean Lee on 2024/10/2.
//

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>

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

  Message message{.type = CONNECT, .string = ""}; // TODO: send token
  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  //  create, connect socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) { std::cerr << "[Error] Failed to create socket (main)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }
  if (connect(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) { std::cerr << "[Error] Unable to connect to host (main)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

  //  ssl context, turn plain socket into ssl connection
  SSL_CTX *ctx = create_context();
  SSL *server_ssl = SSL_new(ctx);
  SSL_set_fd(server_ssl, server_fd);
  if (SSL_connect(server_ssl) <= 0) { std::cerr << "[Error] Unable to SSL_accept (main)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

  std::cout << "[Info] Connected to " << host << ':' << host_main_port << '\n';

  //  send CONNECT message
  if (ssl_send_message(server_ssl, outbuffer, sizeof(outbuffer), message) <= 0){
    std::cerr << "[Warning] Unable to send message (main)\n";
  }
  timer = std::chrono::system_clock::now();

  while (!flag_kill) {
    if (!flag_server_active) {  //  ensure server is alive
      server_response_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);
      if (server_response_duration > std::chrono::seconds(60)) {
        std::cerr << "[Error] Host response timed out\n"; flag_kill = true; break;
      }
    }

    int nbytes;
    try {
      nbytes = ssl_recv_message(server_ssl, inbuffer, sizeof(inbuffer), message);
    } catch (int err) {
      std::cerr << "[Warning] Unable to receive message (main)\n";
    }

    if (nbytes <= 0) {
      SSL_shutdown(server_ssl);
      SSL_free(server_ssl);
      SSL_CTX_free(ctx);
      close(server_fd);
      std::cout << "[Info] Connection to host closed\n";
      flag_kill = true; break;
    } else {
      flag_server_active = true;

      switch (message.type) {
        case HEARTBEAT:
          send_heartbeat_message(server_ssl, outbuffer);
          break;
        case STREAM_PORT:
          std::cout << "[Info] Started streaming to " << readable_host << ':' << message.string << '\n';
          service_thread = std::thread(service_thread_func, std::ref(flag_kill), std::ref(user_id));
          flag_service_thread = true;
          break;
        case REDIRECT:
          user_id.push(message.string);
          break;
      }

      message = {.type = -1, .string = ""};
    }
  }

  if (flag_service_thread) service_thread.join();
  cleanup_openssl();
  return 0;
}
