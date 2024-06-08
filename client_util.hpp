#pragma once

#include <iostream>
#include <thread>

#include "message.hpp"

void heartbeat(int &socket_fd, char *buffer) {
//  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";

  send_message(socket_fd, buffer, message);
  std::cout << "Sent: " << message.type << ", a" << message.string << '\n';
}