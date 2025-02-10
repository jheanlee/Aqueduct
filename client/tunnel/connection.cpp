//
// Created by Jhean Lee on 2024/10/30.
//

#include <thread>
#include <vector>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>

#include "connection.hpp"
#include "message.hpp"
#include "../common/config.hpp"
#include "socket_management.hpp"
#include "../common/console.hpp"

void send_heartbeat_message(SSL *server_ssl, char *buffer) {
  Message message{.type = HEARTBEAT, .string = ""};
  ssl_send_message(server_ssl, buffer, sizeof(buffer), message);
}

static int sha256(const unsigned char *data, size_t data_size, unsigned char *output, size_t output_size) {
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (ctx == nullptr) {
    return -1;
  }
  if (!EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr)) {
    EVP_MD_CTX_free(ctx);
    return -2;
  }
  if (!EVP_DigestUpdate(ctx, data, strlen(reinterpret_cast<const char *>(data)))) {
    EVP_MD_CTX_free(ctx);
    return -3;
  }

  if (output_size < 32) return -4;

  unsigned int hash_len;
  if (!EVP_DigestFinal_ex(ctx, output, &hash_len)) {
    EVP_MD_CTX_free(ctx);
    return -5;
  }

  EVP_MD_CTX_free(ctx);
  return hash_len;
}

static const char base32_encoding_table[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
static int encode_base32(const unsigned char *src, size_t src_size, std::string &output) {
  int output_index = 0;
  int bit_buffer = 0;
  int bit_counter = 0;

  output = "";

  for (int i = 0; i < src_size; i++) {
    bit_buffer = (bit_buffer << 8) | src[i];
    bit_counter += 8;

    while (bit_counter >= 5) {
      output.push_back(base32_encoding_table[(bit_buffer >> (bit_counter - 5)) & 0x1F]);
      output_index++;
      bit_counter -= 5;
    }
  }

  if (bit_counter > 0) {
    output.push_back(base32_encoding_table[(bit_buffer << (5 - bit_counter)) & 0x1F]);
    output_index++;
  }

  while (output_index % 8 != 0) {
    output.push_back('=');
    output_index++;
  }

  return output_index;
}

void send_auth_message(SSL *server_ssl, char *buffer, size_t buffer_size) {
  Message message{.type = AUTHENTICATION, .string = token};
  ssl_send_message(server_ssl, buffer, buffer_size, message);
}

void service_thread_func(std::atomic<bool> &flag_kill, std::queue<std::string> &user_id) {
  std::vector<std::thread> proxy_threads;

  // ##service##
  int service_fd = 0;
  struct sockaddr_in service_addr{.sin_family = AF_INET, .sin_port = htons(local_service_port)};
  inet_pton(AF_INET, local_service, &service_addr.sin_addr);

  while (!flag_kill) {
    while (!flag_kill && user_id.empty()) std::this_thread::yield();

    //  create socket (service)
    service_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (service_fd == -1) {
      console(ERROR, SOCK_CREATE_FAILED, "for service ", "connnection::service");
      cleanup_openssl();
      exit(EXIT_FAILURE);
    }
    
    //  connect (service)
    if (connect(service_fd, (struct sockaddr *) &service_addr, sizeof(service_addr))) {
      console(ERROR, SOCK_CONNECT_FAILED, "for service ", "connection::service");
      cleanup_openssl();
      exit(EXIT_FAILURE);
    }

    console(INFO, CONNECTED_TO_SERVICE, (std::string(local_service) + ':' + std::to_string(local_service_port)).c_str(), "connection::service");

    // ##host##
    int host_fd = 0;
    char buffer[1024] = {0};

    Message redirect_message{.type = REDIRECT, .string = user_id.front()};
    user_id.pop();

    struct sockaddr_in host_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
    inet_pton(AF_INET, host, &host_addr.sin_addr);

    //  create, connect socket (host)
    host_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (host_fd == -1) {
      console(ERROR, SOCK_CREATE_FAILED, "for host ", "connection::service");
      cleanup_openssl();
      exit(EXIT_FAILURE);
    }
    if (connect(host_fd, (struct sockaddr *) &host_addr, sizeof(host_addr))) {
      console(ERROR, SOCK_CONNECT_FAILED, "for host ", "connection::service");
      cleanup_openssl();
      exit(EXIT_FAILURE);
    }

    //  ssl context, turn plain socket into ssl connection
    SSL_CTX *ctx = create_context();
    SSL *host_ssl = SSL_new(ctx);
    SSL_set_fd(host_ssl, host_fd);
    if (SSL_connect(host_ssl) <= 0) {
      console(ERROR, SSL_CONNECT_FAILED, nullptr, "connection::service");
      cleanup_openssl();
      exit(EXIT_FAILURE);
    }

    console(INFO, CONNECTED_FOR_ID, redirect_message.string.c_str(), "connection::service");

    ssl_send_message(host_ssl, buffer, sizeof(buffer), redirect_message);
    proxy_threads.emplace_back(proxy_thread_func, std::ref(flag_kill), host_ssl, host_fd, redirect_message.string, service_fd);
  }

  for (std::thread &t : proxy_threads) t.join();
  close(service_fd);
}

void proxy_thread_func(std::atomic<bool> &flag_kill, SSL *host_ssl, int host_fd, std::string redirect_id, int service_fd) {
  console(INFO, PROXYING_STARTED, redirect_id.c_str(), "connection::proxy");

  int ready_for_call = 0, ready_for_write = 0, write_status = 0;
  ssize_t nbytes = 0;
  char buffer[32768];
  struct pollfd pfds[1];

  while (!flag_kill) {
    //  service -> host
    pfds[0] = {.fd = service_fd, .events = POLLIN | POLLPRI};
    ready_for_call = poll(pfds, 1, timeout_proxy_millisec);
    if (ready_for_call < 0) {
      console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
      break;
    } else if (ready_for_call > 0) {
      //  read from service
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(service_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) {
        console(INFO, CONNECTION_CLOSED_BY_SERVICE, redirect_id.c_str(), "connection::proxy");
        break;
      }

      //  send to host
      pfds[0] = {.fd = SSL_get_fd(host_ssl), .events = POLLOUT | POLLWRBAND};
      ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      while (!flag_kill && ready_for_write == 0) {
        ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      }
      if (ready_for_write < 0) {
        console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
        break;
      }

      write_status = SSL_write(host_ssl, buffer, nbytes);
      if (write_status < 0) {
        if (write_status == -1 && SSL_get_error(host_ssl, write_status) == SSL_ERROR_SYSCALL) {
          console(INFO, CONNECTION_CLOSED_BY_HOST, redirect_id.c_str(), "connection::proxy");
        } else {
          console(ERROR, BUFFER_SEND_ERROR_TO_HOST, redirect_id.c_str(), "connnection::proxy");
        }
        break;
      } else if (write_status == 0) {
        console(INFO, CONNECTION_CLOSED_BY_HOST, redirect_id.c_str(), "connection::proxy");
        break;
      }
    }

    //  host -> service
    pfds[0] = {.fd = SSL_get_fd(host_ssl), .events = POLLIN | POLLPRI};
    ready_for_call = poll(pfds, 1, timeout_proxy_millisec);
    if (ready_for_call < 0) {
      console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
      break;
    } else if (ready_for_call > 0) {
      //  read from host
      memset(buffer, 0, sizeof(buffer));
      nbytes = SSL_read(host_ssl, buffer, sizeof(buffer));
      if (nbytes <= 0) {
        console(INFO, CONNECTION_CLOSED_BY_HOST, redirect_id.c_str(), "connection::proxy");
        break;
      }

      //  send to service
      pfds[0] = {.fd = service_fd, .events = POLLOUT | POLLWRBAND};
      ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      while (!flag_kill && ready_for_write == 0) {
        ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      }
      if (ready_for_write < 0) {
        console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
        break;
      }
      if (send(service_fd, buffer, nbytes, MSG_NOSIGNAL) < 0) {
        if (errno == EPIPE) {
          console(INFO, CONNECTION_CLOSED_BY_SERVICE, redirect_id.c_str(), "connection::proxy");
        } else {
          console(ERROR, BUFFER_SEND_ERROR_TO_SERVICE, redirect_id.c_str(), "connection::proxy");
        }
        break;
      }
    }
  }
  //  clean up
  SSL_shutdown(host_ssl);
  SSL_free(host_ssl);
  close(service_fd); close(host_fd);
  console(INFO, PROXYING_ENDED, redirect_id.c_str(), "connection::proxy");
}