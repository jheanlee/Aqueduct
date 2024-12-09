//
// Created by Jhean Lee on 2024/10/2.
//

#include <iostream>

#include "message.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "opt.hpp"
#include "thread_safe_queue.hpp"

int main(int argc, char *argv[]) {
  opt_handler(argc, argv);

  int socket_fd = 0, status = 0;
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  std::atomic<bool> flag_kill (false);
  ThreadSafeQueue<std::string> user_id;
  bool flag_service_thread = false;

  std::thread service_thread;

  Message message{.type = CONNECT, .string = ""};
  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  //  create socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) { std::cerr << "[Error] Failed to create socket (main)\n"; exit(1); }

  //  connect
  status = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (status == -1) { std::cerr << "[Error] Unable to connect to host (main)\n"; exit(1); }

  try {
    send_message(socket_fd, outbuffer, sizeof(outbuffer), message);
  } catch (int err) {
    std::cerr << "[Warning] Unable to send message (main)\n";
  }
//  std::cout << "To: " << inet_ntoa(server_addr.sin_addr) << ':' << (int)ntohs(server_addr.sin_port) << " " \
//    << "Sent: " << message.type << ", " << message.string << '\n';

  while (!flag_kill) {
    int nbytes;
    try {
      nbytes = recv_message(socket_fd, inbuffer, sizeof(inbuffer), message);
    } catch (int err) {
      std::cerr << "[Warning] Unable to receive message (main)\n";
    }
    if (nbytes <= 0) {
      close(socket_fd);
      std::cout << "[Info] Connection to host closed\n";
      flag_kill = true;
      break;
    } else {
//      std::cout << "Recv: " << message.type << ", " << message.string << '\n';

      switch (message.type) {
        case HEARTBEAT:
          send_heartbeat_message(socket_fd, outbuffer);
          break;
        case STREAM_PORT:
          std::cout << "[Info] Started streaming to " << host << ':' << message.string << '\n';
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

  return 0;
}
