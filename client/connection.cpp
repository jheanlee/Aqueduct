//
// Created by Jhean Lee on 2024/10/30.
//

#include <thread>
#include <vector>
#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>

#include "connection.hpp"
#include "message.hpp"
#include "config.hpp"
#include "socket_management.hpp"

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

void send_auth_message(SSL *server_ssl, char *buffer, size_t buffer_size, std::string &salt) {
  Message message{.type = AUTHENTICATION, .string = token + salt};
  unsigned char hash[EVP_MAX_MD_SIZE];
  int len = sha256(reinterpret_cast<const unsigned char *>(message.string.c_str()), message.string.size(), hash, sizeof(hash));
  if (len < 0) {
    std::cerr << "[Error] unable to hash sha256 \033[2;90m(connection::auth)\033[0m\n\n";
  }
  encode_base32(hash, len, message.string);
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
    if (service_fd == -1) { std::cerr << "[Error] Failed to create socket for service \033[2;90m(connection::service)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }
    
    //  connect (service)
    if (connect(service_fd, (struct sockaddr *) &service_addr, sizeof(service_addr))) { std::cerr << "[Error] Failed to connect to service \033[2;90m(connection::service)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    std::cout << "[Info] Connected to service \033[2;90m(connection::service)\033[0m\n";

    // ##host##
    int host_fd = 0;
    char buffer[1024] = {0};

    Message redirect_message{.type = REDIRECT, .string = user_id.front()};
    user_id.pop();

    struct sockaddr_in host_addr{.sin_family = AF_INET, .sin_port = htons(host_main_port)};
    inet_pton(AF_INET, host, &host_addr.sin_addr);

    //  create, connect socket (host)
    host_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (host_fd == -1) { std::cerr << "[Error] Failed to create socket for host \033[2;90m(connection::service)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }
    if (connect(host_fd, (struct sockaddr *) &host_addr, sizeof(host_addr))) { std::cerr << "[Error] Failed to connect to host \033[2;90m(connection::service)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    //  ssl context, turn plain socket into ssl connection
    SSL_CTX *ctx = create_context();
    SSL *host_ssl = SSL_new(ctx);
    SSL_set_fd(host_ssl, host_fd);
    if (SSL_connect(host_ssl) <= 0) { std::cerr << "[Error] Unable to SSL_connect \033[2;90m(connection::service)\033[0m\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    std::cout << "[Info] Connected to host for redirect id: "<< redirect_message.string << " \033[2;90m(connection::service)\033[0m\n";

    ssl_send_message(host_ssl, buffer, sizeof(buffer), redirect_message);
    proxy_threads.emplace_back(proxy_thread_func, std::ref(flag_kill), host_ssl, host_fd, host_addr, service_fd);
  }

  for (std::thread &t : proxy_threads) t.join();
  close(service_fd);
}

void proxy_thread_func(std::atomic<bool> &flag_kill, SSL *host_ssl, int host_fd, sockaddr_in host_addr, int service_fd) {
  std::cout << "[Info] Proxying started \033[2;90m(connection::proxy)\033[0m\n";

  int ready_for_call = 0;
  ssize_t nbytes = 0;
  char buffer[32768];
  fd_set read_fd;
  timeval timev = {.tv_sec = select_timeout_proxy_sec, .tv_usec = select_timeout_proxy_millisec};

  while (!flag_kill) {
    // service -> host
    FD_ZERO(&read_fd); FD_SET(service_fd, &read_fd);
    timev = {.tv_sec = select_timeout_proxy_sec, .tv_usec = select_timeout_proxy_millisec};

    ready_for_call = select(service_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select \033[2;90m(connection::proxy)\033[0m\n"; break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(service_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) { std::cout << "[Info] Connection has been closed by service \033[2;90m(connection::proxy)\033[0m\n"; break; }
      if (SSL_write(host_ssl, buffer, nbytes) < 0) { std::cerr << "[Warning] Failed to send buffer to host \033[2;90m(connection:proxy)\033[0m\n"; break; }
    }

    // host -> service
    FD_ZERO(&read_fd); FD_SET(host_fd, &read_fd);
    timev = {.tv_sec = select_timeout_proxy_sec, .tv_usec = select_timeout_proxy_millisec};

    ready_for_call = select(SSL_get_fd(host_ssl) + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = SSL_read(host_ssl, buffer, sizeof(buffer));
      if (nbytes <= 0) { std::cout << "[Info] Connection has been closed by host \033[2;90m(connection::proxy)\033[0m\n"; break; }
      if (send(service_fd, buffer, nbytes, 0) < 0) { std::cerr << "[Warning] Error occurred while sending buffer to service \033[2;90m(connection:proxy)\033[0m\n"; }
    }
  }
  SSL_shutdown(host_ssl);
  SSL_free(host_ssl);
  close(service_fd); close(host_fd);
  std::cout << "[Info] Proxying ended \033[2;90m(connection::proxy)\033[0m\n";
}