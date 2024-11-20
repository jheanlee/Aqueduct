//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_MESSAGE_HPP
  #define TUNNEL_MESSAGE_HPP

  #include <string>
  #include <cstring>
  #include <exception>
  #include <iostream>

  #include <sys/socket.h>
  #include <unistd.h>

  #define CONNECT     '0'
  #define HEARTBEAT   '1'
  #define STREAM_PORT '2'
  #define REDIRECT    '3'

  class Message {
    public:
    char type;
    std::string string;

    void load(char *buffer);
    void dump(char *buffer) const;

  };

  int send_message(int &socket_fd, char *buffer, Message &message);
  int recv_message(int &socket_fd, char *buffer, Message &message);

  int read_message_non_block(fd_set &read_fd, int &socket_fd, timeval &timev, char *buffer, Message &message);

#endif //TUNNEL_MESSAGE_HPP
