//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_MESSAGE_HPP
#define TUNNEL_MESSAGE_HPP

#include <openssl/ssl.h>

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

int send_message(int &socket_fd, char *buffer, size_t buffer_size, Message &message);
int ssl_send_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);
int recv_message(int &socket_fd, char *buffer, size_t buffer_size, Message &message);
int ssl_recv_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);

int read_message_non_block(fd_set &read_fd, int &socket_fd, timeval &timev, char *buffer, size_t buffer_size, Message &message);
int ssl_read_message_non_block(SSL *ssl, char *buffer, size_t buffer_size, Message &message);

#endif //TUNNEL_MESSAGE_HPP
