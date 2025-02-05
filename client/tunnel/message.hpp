//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef TUNNEL_MESSAGE_HPP
  #define TUNNEL_MESSAGE_HPP

  #include <openssl/ssl.h>

  #define MESSAGE_MAX_STRING_SIZE 127

  #define CONNECT         '0'
  #define HEARTBEAT       '1'
  #define STREAM_PORT     '2'
  #define REDIRECT        '3'
  #define AUTHENTICATION  '4'
  #define AUTH_SUCCESS    '5'
  #define AUTH_FAILED     '6'
  #define DB_ERROR        '7'

  class Message {
  public:
    char type;
    std::string string;

    void load(char *buffer);
    void dump(char *buffer) const;

  };

  int ssl_send_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);
  int ssl_recv_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);

#endif //TUNNEL_MESSAGE_HPP
