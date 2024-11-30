//
// Created by Jhean Lee on 2024/10/2.
//

#include "message.hpp"

void Message::load(char *buffer) {
  if (strlen(buffer) == 0) throw -1;
  if (strlen(buffer) > 64) throw -1;

  type = buffer[0];
  string = std::string(buffer + 1);
}

void Message::dump(char *buffer) const {
  if (type == '\0') throw -1;
  if (string.size() > 63) throw -1;

  buffer[0] = type;
  strcat(buffer, string.c_str());
}

int send_message(int &socket_fd, char *buffer, size_t buffer_size, Message &message) {
  std::lock_guard<std::mutex> lock(shared_resources::send_mutex);

  try {
    std::memset(buffer, '\0', buffer_size);
    message.dump(buffer);
  } catch (int err) {
    std::cerr << "[message.cpp] Error occurred in Message::dump() \n";
    return -1;
  }

  send(socket_fd, buffer, strlen(buffer), 0);
  return 0;
}

int recv_message(int &socket_fd, char *buffer, size_t buffer_size, Message &message) {
  std::memset(buffer, '\0', buffer_size);
  int nbytes = recv(socket_fd, buffer, buffer_size, 0);
  if (nbytes <= 0) return nbytes;

  try {
    message.load(buffer);
  } catch (int err) {
    std::cerr << "[message.cpp] Error occurred in Message::load() \n" << buffer;
    return -1;
  }

  return nbytes;
}

int read_message_non_block(fd_set &read_fd, int &socket_fd, timeval &timev, char *buffer, size_t buffer_size, Message &message) {
  FD_ZERO(&read_fd);
  FD_SET(socket_fd, &read_fd);
  timev.tv_sec = 0; timev.tv_usec = 50;

  int ready_for_call = select(socket_fd + 1, &read_fd, nullptr, nullptr, &timev);

  if (ready_for_call < 0) {
    std::cerr << "[message.cpp] Error occurred in select() \n";
    return -1;
  } else if (ready_for_call == 0) {
    return 0;
  } else {
    int recv_status = recv_message(socket_fd, buffer, buffer_size, message); // recv_status = nbytes
    if (recv_status == 0) return -1;
    return recv_status;
  }
  return -2;
}