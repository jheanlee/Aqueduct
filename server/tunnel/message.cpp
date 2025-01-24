//
// Created by Jhean Lee on 2024/10/2.
//
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "shared.hpp"
#include "message.hpp"
#include "config.hpp"

void Message::load(char *buffer) {
  if (strlen(buffer) == 0) throw -1;
  if (strlen(buffer) > MESSAGE_MAX_STRING_SIZE + 1) throw -1;

  type = buffer[0];
  string = std::string(buffer + 1);
}

void Message::dump(char *buffer) const {
  if (type == '\0') throw -1;
  if (string.size() > MESSAGE_MAX_STRING_SIZE) throw -1;

  buffer[0] = type;
  strcat(buffer, string.c_str());
}

int ssl_send_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message) {
  std::lock_guard<std::mutex> lock(shared_resources::ssl_send_mutex);

  try {
    std::memset(buffer, '\0', buffer_size);
    message.dump(buffer);
  } catch (int err) {
    std::cerr << "[Warning] Unable to dump message \033[2;90m(message)\033[0m\n";
    return -1;
  }

  return SSL_write(ssl, buffer, buffer_size);
}

int ssl_recv_message(SSL *ssl, char *buffer, size_t buffer_size, Message &message) {
  std::memset(buffer, '\0', buffer_size);
  int nbytes = SSL_read(ssl, buffer, buffer_size);
  if (nbytes <= 0) return nbytes;

  try {
    message.load(buffer);
  } catch (int err) {
    std::cerr << "[Warning] Unable to load message \033[2;90m(message)\033[0m\n" << buffer;
    return -1;
  }

  return nbytes;
}

int ssl_read_message_non_block(SSL *ssl, fd_set &read_fd, timeval &timev, char *buffer, size_t buffer_size, Message &message) {
  FD_ZERO(&read_fd);
  FD_SET(SSL_get_fd(ssl), &read_fd);
  timev.tv_sec = select_timeout_session_sec; timev.tv_usec = select_timeout_session_millisec;

  int ready_for_call = select(SSL_get_fd(ssl) + 1, &read_fd, nullptr, nullptr, &timev);

  if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select \033[2;90m(message)\033[0m\n";
      return -1;
  } else if (ready_for_call == 0) {
    return 0;
  } else {
    int recv_status = ssl_recv_message(ssl, buffer, buffer_size, message);
    if (recv_status == 0) return -1;
    return recv_status;
  }
  return -2;
}