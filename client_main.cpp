#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>

#include "common/message.hpp"
#include "client/client_util.hpp"
#include "client/client_config.hpp"

int host_stream_port;

int main() {
  std::vector<std::thread> stream_service_threads;

  int control_socket_fd;
  int control_socket_status;

  // socket creation
  control_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (control_socket_fd == -1) { std::cerr << "Failed to create socket.\n"; exit(1); }

  // server address
  struct sockaddr_in server_addr;

  server_addr.sin_family = AF_INET;
  // inet_aton(host, &server_addr.sin_addr);
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(host_main_port);

  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;

  // connect
  control_socket_status = connect(control_socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  if (control_socket_status == -1) { std::cerr << "Connection error.\n"; exit(1); }

  message.type = CONNECT;
  message.string = "";
  try {
    send_message(control_socket_fd, outbuffer, message);
  } catch (int err) {
    std::cerr << "Error sending message.\n";
  }
  std::cout << "Sent: " << outbuffer << '\n';

  while (true) {

    // receive buffer
    int nbytes = 0;
    try {
      nbytes = recv_message(control_socket_fd, inbuffer, message);
    } catch (int err) {
      std::cerr << "Error receiving message.\n";
    }

    if (nbytes <= 0) {
      close(control_socket_fd);
      std::cout << "Connection closed.\n";
      break;
    }

    std::cout << "Recv: " << message.type << ", " << message.string << '\n';

    if (message.type == HEARTBEAT) heartbeat(control_socket_fd, outbuffer);

    if (message.type == REDIRECT) {
      host_stream_port = std::stoi(message.string);
      std::cout << "Started forwarding from " << host << ':' << message.string << '\n';
      stream_service_threads.emplace_back(stream_service, id); //TODO
      //TODO
    }


  }

  for (std::thread &t : stream_service_threads) t.join();

  close(control_socket_fd);
  return 0;
}