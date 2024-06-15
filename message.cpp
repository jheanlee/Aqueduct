#include "message.hpp"


void Message::load(char *buffer) {
  if (strlen(buffer) > 64) throw -1;
  if (strlen(buffer) == 0) throw -1;

  type = buffer[0];
  string = std::string(buffer + 1);
}

void Message::dump(char *buffer) {
  if (type == '\0') throw -1;
  if (string.size() > 63) throw -1;

  buffer[0] = type;
  strcat(buffer, string.c_str());
}


int send_message(int socket, char *buffer, Message &message) {
  try {
    message.dump(buffer);
  } catch (int err) {
    throw(err);
  }

  send(socket, buffer, strlen(buffer), 0);
  return 0;
}

int recv_message(int socket, char *buffer, Message &message) {
  int nbytes = recv(socket, buffer, sizeof(buffer), 0);
  if (nbytes <= 0) return nbytes;

  try {
    message.load(buffer);
  } catch (int err) {
    throw(err);
  }
  
  return nbytes;
}