//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_MESSAGE_HPP
  #define TUNNEL_MESSAGE_HPP

  #include <openssl/ssl.h>
  #include <poll.h>

  #define MESSAGE_MAX_STRING_SIZE 127

  #define CONNECT         '0'
  #define HEARTBEAT       '1'
  #define STREAM_PORT     '2'
  #define NO_PORT         '3'
  #define REDIRECT        '4'
  #define AUTHENTICATION  '5'
  #define AUTH_SUCCESS    '6'
  #define AUTH_FAILED     '7'
  #define DB_ERROR        '8'

  class Message {
  public:
    char type;
    std::string string;

    void load(char *buffer);
    void dump(char *buffer) const;
  };

  int ssl_send_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);
  int ssl_recv_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);
  int ssl_read_message_non_block(SSL *ssl, pollfd *pfds, char *buffer, size_t buffer_size, Message &message);

#endif //TUNNEL_MESSAGE_HPP
