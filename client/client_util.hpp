#ifndef TUNNEL_CLIENT_UTIL_HPP
  #define TUNNEL_CLIENT_UTIL_HPP

  #include <iostream>
  #include <thread>

  #include "../common/message.hpp"

  void heartbeat(int &socket_fd, char *buffer);
#endif