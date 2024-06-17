#ifndef TUNNEL_MESSAGE_HPP
  #define TUNNEL_MESSAGE_HPP

  #include <string>
  #include <cstring>

  #include <sys/socket.h>

  #define CONNECT     '0'
  #define HEARTBEAT   '1'
  #define REDIRECT     '2'

/*
  byte 0: message type (CONNECT, HEARTBEAT, etc.)
  byte 1-63: message
*/

class Message {
  public:
  
  char type;
  std::string string;

  void load(char *buffer);

  void dump(char *buffer);
};

int send_message(int socket, char *buffer, Message &message);

int recv_message(int socket, char *buffer, Message &message);

#endif