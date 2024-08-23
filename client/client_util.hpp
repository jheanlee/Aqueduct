#ifndef TUNNEL_CLIENT_UTIL_HPP
  #define TUNNEL_CLIENT_UTIL_HPP

  #include <iostream>
  #include <thread>
  #include <atomic>

  #include <unistd.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>

  #include "../common/message.hpp"
  #include "client_config.hpp"


void heartbeat(int &socket_fd, char *outbuffer);

void stream_service(int id);

#endif