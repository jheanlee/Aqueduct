#pragma once

#include <iostream>

#include <unistd.h>

#include "message.hpp"

void heartbeat(int &client_fd) {
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";
  while (true) {
    try {
      send_message(client_fd, outbuffer, message);
    } catch (int err) {
      std::cerr << "Error sending message.\n";
    }
  }
}


void receive(int &client_fd) {
  int nbytes;
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;

  while (true) {
    try {
      nbytes = recv_message(client_fd, inbuffer, message); // number of bytes received
    } catch (int err) {
      std::cerr << "Error receiving message.\n";
    }

    // nbytes == 0: client closed connection; nbytes == -1: error
    if (nbytes <= 0) { 
      close(client_fd);
      std::cout << "Connection closed.\n";
      break;
    }

    std::cout << "Received: " << message.type << ',' << message.string << '\n';
  }
}