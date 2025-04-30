//
// Created by Jhean Lee on 2024/10/2.
//

#ifndef AQUEDUCT_MESSAGE_HPP
  #define AQUEDUCT_MESSAGE_HPP

  #include <openssl/ssl.h>
  #include <poll.h>
  #include <mutex>

  #define MESSAGE_MAX_STRING_SIZE 126

  #define CONNECT         '0'
  #define HEARTBEAT       '1'
  #define STREAM_PORT     '2'
  #define NO_PORT         '3'
  #define REDIRECT        '4'
  #define AUTHENTICATION  '5'
  #define AUTH_SUCCESS    '6'
  #define AUTH_FAILED     '7'
  #define DB_ERROR        '8'
  
  #define API_HEARTBEAT   'B'
  #define API_EXIT        'C'
  #define API_GET_SERVICE_INFO    'D'
  #define API_GET_CURRENT_CLIENTS 'E'
  #define API_GENERATE_NEW_TOKEN  'F'


  class Message {
  public:
    char type;
    std::string string;

    void load(char *buffer);
    void load(char *buffer, size_t limit);
    void dump(char *buffer) const;
    void dump_large(char *buffer, size_t buffer_size) const;
  };

  int send_message(int &fd, char *buffer, size_t buffer_size, Message &message, std::mutex &send_mutex);
  int send_large_message(int &fd, char *buffer, size_t buffer_size, Message &message, std::mutex &send_mutex);
  int recv_message(int &fd, char *buffer, size_t buffer_size, Message &message);
  int recv_message(int &fd, char *buffer, size_t buffer_size, Message &message, size_t limit);
  int read_message_non_block(int &fd, pollfd *pfds, char *buffer, size_t buffer_size, Message &message);
  int read_message_non_block(int &fd, pollfd *pfds, char *buffer, size_t buffer_size, Message &message, size_t limit);
  int ssl_send_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message, std::mutex &send_mutex);
  int ssl_recv_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message);
  int ssl_read_message_non_block(SSL *ssl, pollfd *pfds, char *buffer, size_t buffer_size, Message &message);

#endif //AQUEDUCT_MESSAGE_HPP