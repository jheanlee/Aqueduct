#pragma once

#include <iostream>
#include <thread>

#include "message.hpp"

void heartbeat(int &socket_fd) {
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";
}