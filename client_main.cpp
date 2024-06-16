#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common/message.hpp"
#include "client/client_util.hpp"

const char *host = "0.0.0.0";
int main_port = 3000;

int main() {
  int socket_fd;
  int status;

  // socket creation
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (socket_fd == -1) { std::cerr << "Failed to create socket.\n"; exit(1); }

  // server address
  struct sockaddr_in server_addr;

  server_addr.sin_family = AF_INET;
  // inet_aton(host, &server_addr.sin_addr);
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(main_port);

  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;

  // connect
  status = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  if (status == -1) { std::cerr << "Connection error.\n"; exit(1); }

  message.type = CONNECT;
  message.string = "";
  try {
    send_message(socket_fd, outbuffer, message);
  } catch (int err) {
    std::cerr << "Error sending message.\n";
  }
  std::cout << "Sent: " << outbuffer << '\n';

  while (true) {

    // receive buffer
    int nbytes = 0;
    try {
      nbytes = recv_message(socket_fd, inbuffer, message);
    } catch (int err) {
      std::cerr << "Error receiving message.\n";
    }

    if (nbytes <= 0) {
      close(socket_fd);
      std::cout << "Connection closed.\n";
      break;
    }

    std::cout << "Recv: " << message.type << ", " << message.string << '\n';

    if (message.type == HEARTBEAT) heartbeat(socket_fd, inbuffer);


  }

  close(socket_fd);
  return 0;
}