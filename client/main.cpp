//
// Created by Jhean Lee on 2024/10/2.
//

#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include <unistd.h>
#include <poll.h>
#if defined(__clang__) && defined(__APPLE__)
  #include <os/log.h>
#else
  #include <sys/syslog.h>
#endif

#include "tunnel/message.hpp"
#include "common/config.hpp"
#include "tunnel/connection.hpp"
#include "common/opt.hpp"
#include "tunnel/socket_management.hpp"
#include "common/console.hpp"
#include "common/signal_handler.hpp"
#include "common/shared.hpp"

int main(int argc, char *argv[]) {
  register_signal();
  #if defined(__OS_LOG_H__)
    shared_resources::os_log_aqueduct = os_log_create("cloud.drizzling.aqueduct", "core");
  #else
    openlog("aqueduct-client", LOG_PID | LOG_CONS, LOG_DAEMON);
  #endif
  opt_handler(argc, argv);
  init_openssl();

  bool flag_service_thread = false, flag_server_active = false;
  int server_fd = 0, status = 0, nbytes = 0;
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  std::atomic<bool> flag_kill(false);
  std::queue<std::string> user_id;
  std::chrono::system_clock::time_point timer;
  std::chrono::seconds server_response_duration;
  std::mutex send_mutex;

  std::thread service_thread;

  struct pollfd pfds[1];

  Message message{.type = CONNECT, .string = ""};
  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  //  create, connect socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    console(CRITICAL, SOCK_CREATE_FAILED, nullptr, "main");
    signal_handler(EXIT_FAILURE);
  }
  if (connect(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    console(CRITICAL, SOCK_CONNECT_FAILED, "to host", "main");
    signal_handler(EXIT_FAILURE);
  }

  //  ssl context, turn plain socket into ssl connection
  SSL_CTX *ctx = create_context();
  SSL *server_ssl = SSL_new(ctx);
  SSL_set_fd(server_ssl, server_fd);
  if (SSL_connect(server_ssl) <= 0) {
    console(CRITICAL, SSL_CONNECT_FAILED, nullptr, "main");
    signal_handler(EXIT_FAILURE);
  }

  console(NOTICE, CONNECTED_TO_HOST, (std::string(host) + ':' + std::to_string(host_main_port)).c_str(), "main");

  //  send CONNECT message
  if (ssl_send_message(server_ssl, outbuffer, sizeof(outbuffer), message, send_mutex) <= 0){
    console(ERROR, MESSAGE_SEND_FAILED, (std::string(host) + ':' + std::to_string(host_main_port)).c_str(), "main");
  }
  timer = std::chrono::system_clock::now();

  while (!flag_kill) {
    if (!flag_server_active) {  //  ensure server is responsive
      server_response_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);
      if (server_response_duration > std::chrono::seconds(60)) {
        console(CRITICAL, HOST_RESPONSE_TIMEOUT, nullptr, "main");
        flag_kill = true;
        break;
      }
    }

    //  reading message without blocking
    pfds[0] = {.fd = server_fd, .events = POLLIN | POLLPRI};
    status = poll(pfds, 1, timeout_session_millisec);

    if (status < 0) {
      console(CRITICAL, SOCK_POLL_ERR, nullptr, "main");
      flag_kill = true;
      break;
    } else if (status > 0) {
      nbytes = ssl_recv_message(server_ssl, inbuffer, sizeof(inbuffer), message);
    } else {
      continue;
    }

    if (nbytes <= 0) {
      SSL_shutdown(server_ssl);
      SSL_free(server_ssl);
      SSL_CTX_free(ctx);
      close(server_fd);
      console(NOTICE, CONNECTION_CLOSED, (std::string(host) + ':' + std::to_string(host_main_port)).c_str(), "main");
      flag_kill = true;
      break;
    } else {
      flag_server_active = true;
      switch (message.type) {
        case HEARTBEAT:
          send_heartbeat_message(server_ssl, outbuffer, send_mutex);
          break;
        case STREAM_PORT:
          console(NOTICE, STREAM_PORT_OPENED, (std::string(readable_host) + ':' + message.string).c_str(), "main");
          service_thread = std::thread(service_thread_func, std::ref(flag_kill), std::ref(user_id));
          flag_service_thread = true;
          break;
        case NO_PORT:
          flag_kill = true;
          console(CRITICAL, NO_PORTS_AVAILABLE, nullptr, "main");
          break;
        case REDIRECT:
          user_id.push(message.string);
          break;
        case AUTHENTICATION:
          send_auth_message(server_ssl, outbuffer, sizeof(outbuffer), send_mutex);
          console(DEBUG, AUTHENTICATION_REQUEST_SENT, nullptr, "main");
          break;
        case AUTH_SUCCESS:
          console(INFO, AUTHENTICATION_SUCCESS, nullptr, "main");
          break;
        case AUTH_FAILED:
          flag_kill = true;
          console(CRITICAL, AUTHENTICATION_FAILED, nullptr, "main");
          break;
        case DB_ERROR:
          flag_kill = true;
          console(CRITICAL, SERVER_DATABASE_ERROR, nullptr, "main");
          break;
      }

      message = {.type = '\0', .string = ""};
    }
  }

  if (flag_service_thread) service_thread.join();
  signal_handler(0);
  return 0;
}
