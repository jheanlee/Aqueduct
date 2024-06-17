#include "client_util.hpp"

void heartbeat(int &socket_fd, char *outbuffer) {
//  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";

  send_message(socket_fd, outbuffer, message);
  std::cout << "Sent: " << message.type << ", " << message.string << '\n';
}