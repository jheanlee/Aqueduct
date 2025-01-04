//
// Created by Jhean Lee on 2024/10/2.
//
#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <cstring>

#include <unistd.h>
#include <uuid/uuid.h>

#include "socket_management.hpp"
#include "connection.hpp"
#include "config.hpp"
#include "message.hpp"
#include "shared.hpp"

//void control_thread_func(std::atomic<bool> &flag_kill) {
//  int socket_fd = 0;
//  std::unordered_map<std::string, std::pair<int, sockaddr_in>> external_user_id_map; // id, {fd, addr}
//  std::vector<std::thread> client_session_threads;
//
//  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(control_port)}, client_addr{};
//  inet_pton(AF_INET, host, &server_addr.sin_addr);
//
//  socket_fd = create_socket(server_addr);
//  std::cout << "[Info] Waiting for connection... (control)\n";
//
//  int client_fd;
//  socklen_t client_addrlen = sizeof(client_addr);
//  while (!flag_kill) {
//    client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addrlen);
//    if (client_fd < 0) { std::cerr << "[Error] Unable to accept (control)\n"; exit(EXIT_FAILURE); }
//
//    std::cout << "[Info] Connection from: " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " (control)\n";
//
//    std::thread session_thread(session_thread_func, client_fd, client_addr, ref(external_user_id_map));
//    session_thread.detach();
//  }
//
//  close(socket_fd);
//}

void ssl_control_thread_func(std::atomic<bool> &flag_kill) {
  int socket_fd = 0;
  SSL_CTX *ctx;
  std::unordered_map<std::string, std::pair<int, sockaddr_in>> external_user_id_map; // id, {external_user_fd, addr}

  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(ssl_control_port)}, client_addr{};
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  //  create SSL context
  ctx = create_context();
  config_context (ctx);

  //  create, bind, listen socket
  socket_fd = create_socket(server_addr);

  std::cout << "[Info] Waiting for connection... (ssl_control)\n";

  //  accept connections from client, turn plain socket into SSL connections
  int client_fd = 0;
  socklen_t client_addrlen = sizeof(client_addr);
  while (!flag_kill) {
    SSL *client_ssl;
    client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addrlen);

    if (client_fd < 0) { std::cerr << "[Error] Unable to accept (ssl_control)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    client_ssl = SSL_new(ctx);
    SSL_set_fd(client_ssl, client_fd);
    if (SSL_accept(client_ssl) <= 0) { std::cerr << "[Error] Unable to SSL_accept (ssl_control)\n"; cleanup_openssl(); exit(EXIT_FAILURE); }

    std::cout << "[Info] Connection from: " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " (client_ssl control)\n";
    std::thread session_thread(ssl_session_thread_func, client_fd, client_ssl, client_addr, std::ref(external_user_id_map));
    session_thread.detach();
  }

  //  clean up
  close(socket_fd);
  SSL_CTX_free(ctx);
}

//void heartbeat_thread_func(int &client_fd, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill) {
//  char outbuffer[1024] = {0};
//  Message heartbeat_message = {.type = HEARTBEAT, .string = ""};
//
//  std::chrono::system_clock::time_point timer;
//  std::chrono::seconds heartbeat_duration;
//
//  while (!flag_kill) {
//    try {
//      send_message(client_fd, outbuffer, sizeof(outbuffer), heartbeat_message);
//    } catch (int err) {
//      std::cerr << "[Warninng] Unable to send message (connection::heartbeat)\n";
//    }
//
//    //  start timing
//    timer = std::chrono::system_clock::now();
//    flag_heartbeat_received = false;
//
//    //  wait for heartbeat
//    while (!flag_heartbeat_received) {
//      heartbeat_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);
//
//      if (heartbeat_duration > std::chrono::seconds(heartbeat_timeout_sec)) {
//        std::cout << "[Info] Heartbeat timeout: " << inet_ntoa(client_addr.sin_addr) << ':' << (int) ntohs(client_addr.sin_port) << '\n';
//        flag_kill = true; return;
//      }
//    }
//
//    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
//  }
//}

void ssl_heartbeat_thread_func(SSL *client_ssl, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill) {
  char outbuffer[1024] = {0};
  Message heartbeat_message = {.type = HEARTBEAT, .string = ""};

  std::chrono::system_clock::time_point timer;
  std::chrono::seconds heartbeat_duration;

  while (!flag_kill) {
    //  send heartbeat message
    try {
      ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), heartbeat_message);
    } catch (int err) {
      std::cerr << "[Warninng] Unable to send message (connection::heartbeat)\n";
    }

    //  start timing
    timer = std::chrono::system_clock::now();
    flag_heartbeat_received = false;

    //  wait for heartbeat
    while (!flag_heartbeat_received) {
      heartbeat_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);

      if (heartbeat_duration > std::chrono::seconds(heartbeat_timeout_sec)) {
        std::cout << "[Info] Heartbeat timeout: " << inet_ntoa(client_addr.sin_addr) << ':' << (int) ntohs(client_addr.sin_port) << '\n';
        flag_kill = true; return;
      }
    }

    //  sleep
    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
  }
}

//void session_thread_func(int client_fd, sockaddr_in client_addr, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id) {
//  bool flag_first_msg = false, flag_connect_type = false, flag_proxy_type = false;
//  std::atomic<bool> flag_heartbeat_received(false), flag_kill(false);
//  int recv_status = 0;
//  char inbuffer[1024] = {0};
//  Message message {.type = -1, .string = ""};
//
//  fd_set read_fds;
//  timeval timev = {.tv_sec = 0, .tv_usec = 50};
//
//  std::thread heartbeat_thread;
//  std::thread proxy_service_port_thread; // user-proxyserver connection, stream to service(client)
//  std::thread proxy_thread;
//
//  std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
//  std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);
//
//  while (!flag_kill && (flag_first_msg || duration <= std::chrono::seconds(first_message_timeout_sec))) {
//    if (!flag_first_msg) duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);
//    recv_status = read_message_non_block(read_fds, client_fd, timev, inbuffer, sizeof(inbuffer), message);
//    if (recv_status < 0) {
//      flag_kill = true;
//    } else if (recv_status > 0){
//      switch (message.type) {
//        case CONNECT:
//          flag_first_msg = true; flag_connect_type = true;
//          heartbeat_thread = std::thread(heartbeat_thread_func, std::ref(client_fd), std::ref(client_addr), std::ref(flag_heartbeat_received), std::ref(flag_kill));
//          proxy_service_port_thread = std::thread(proxy_service_port_thread_func, std::ref(flag_kill), std::ref(external_user_id), std::ref(client_fd), std::ref(client_addr));
//          break;
//        case HEARTBEAT:
//          flag_heartbeat_received = true;
//          break;
//        case REDIRECT:
//          flag_first_msg = true;
//
//          if (external_user_id.find(message.string) != external_user_id.end()) {
//            std::unique_lock<std::mutex> lock(shared_resources::external_user_mutex);
//            std::pair<int, sockaddr_in> external_user = external_user_id[message.string];
//            external_user_id.erase(message.string);
//            lock.unlock();
//
//            proxy_thread = std::thread(proxy_thread_func, client_fd, external_user.first, std::ref(flag_kill));
//            flag_proxy_type = true;
//          } else flag_kill = true;
//          goto proxy;
//      }
//    }
//  }
//  flag_kill = true;
//  proxy:
//  if (flag_connect_type) { heartbeat_thread.join(); proxy_service_port_thread.join(); }
//  if (flag_proxy_type) { proxy_thread.join(); }
//
//  close(client_fd);
//  std::cout << "[Info] Connection closed: " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << '\n';
//}

void ssl_session_thread_func(int client_fd, SSL *client_ssl, sockaddr_in client_addr, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id) {
  bool flag_first_msg = false, flag_connect_type = false, flag_proxy_type = false;
  std::atomic<bool> flag_heartbeat_received(false), flag_kill(false);
  int recv_status = 0;
  char inbuffer[1024] = {0};
  Message message {.type = -1, .string = ""};

  std::thread heartbeat_thread; //  ensure connection is alive
  std::thread proxy_service_port_thread; // external_user <-> proxy_server connection, sends id to service(client)
  std::thread proxy_thread; //  external_user<->proxy_server <=== read/write i/o ===> proxy_server<->client(service)

  std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
  std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

  while (!flag_kill && (flag_first_msg || duration <= std::chrono::seconds(first_message_timeout_sec))) {

    //  if client does not send message to server within <first_message_timeout_sec>, close the connection
    if (!flag_first_msg) duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

    recv_status = ssl_read_message_non_block(client_ssl, inbuffer, sizeof(inbuffer), message);
    if (recv_status < 0) {  //  error or connection closed
      flag_kill = true;
    } else if (recv_status > 0) {
      switch (message.type) {
        case CONNECT:
          flag_first_msg = true; flag_connect_type = true;

          //  ensure connection is alive
          heartbeat_thread = std::thread(ssl_heartbeat_thread_func, client_ssl, std::ref(client_addr), std::ref(flag_heartbeat_received), std::ref(flag_kill));
          //  external_user <-> proxy_server connection, sends id to service(client)
          proxy_service_port_thread = std::thread(proxy_service_port_thread_func, std::ref(flag_kill), std::ref(external_user_id), client_ssl, std::ref(client_addr));
          break;
        case HEARTBEAT:

          //  sets heartbeat flag to true (used by heartbeat_thread)
          flag_heartbeat_received = true;
          break;
        case REDIRECT:
          flag_first_msg = true;

          if (external_user_id.find(message.string) != external_user_id.end()) {
            //  open proxy_thread for client-accepted external_user (by id)
            std::unique_lock<std::mutex> lock(shared_resources::external_user_mutex);
            std::pair<int, sockaddr_in> external_user = external_user_id[message.string];
            external_user_id.erase(message.string);
            lock.unlock();

            //  external_user<->proxy_server <=== read/write io ===> proxy_server<->client(service)
            proxy_thread = std::thread(proxy_thread_func, client_ssl, external_user.first, std::ref(flag_kill));
            flag_proxy_type = true;
          } else flag_kill = true;
          goto proxy;
      }
    }
  }
  flag_kill = true;
  proxy:
  if (flag_connect_type) { heartbeat_thread.join(); proxy_service_port_thread.join(); }
  if (flag_proxy_type) { proxy_thread.join(); }

  //  clean up
  SSL_shutdown(client_ssl);
  SSL_free(client_ssl);
  close(client_fd);
  std::cout << "[Info] Connection closed: " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << '\n';
}

void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::unordered_map<std::string, std::pair<int, sockaddr_in>> &external_user_id, SSL *client_ssl, sockaddr_in &client_addr) {
  int service_proxy_fd = 0, proxy_port = 0, external_user_fd = 0;
  char outbuffer[1024] = {0}, uuid_str[37] = {0};

  struct sockaddr_in server_proxy_addr = {.sin_family = AF_INET}, external_user_addr = {.sin_family = AF_INET};
  inet_pton(AF_INET, host, &server_proxy_addr.sin_addr);

  uuid_t uuid;
  Message message{.type = STREAM_PORT, .string = ""};

  //  create socket
  service_proxy_fd = create_socket(server_proxy_addr);

  //  set proxy port
  std::unique_lock<std::mutex> ports_assign_lock(shared_resources::ports_mutex);
  if (proxy_ports_available.empty()) { std::cerr << "[Warning] No ports available (connection::proxy_service)\n"; flag_kill = true; return; }
  while (!proxy_ports_available.empty()) {
    server_proxy_addr.sin_port = htons(proxy_ports_available.front());

    if (bind_socket(service_proxy_fd, server_proxy_addr) < 0) { proxy_ports_available.pop(); continue; }

    std::cout << "[Info] Opened proxy port at: " << proxy_ports_available.front() << '\n';
    proxy_port = proxy_ports_available.front();
    proxy_ports_available.pop();
    break;
  }
  ports_assign_lock.unlock();

  //  send the port to which the service is streamed to client (external users can connect to this port)
  message.string = std::to_string((int) ntohs(server_proxy_addr.sin_port));
  ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message);

  //  accept external connections (end users)
  while (!flag_kill) {
    socklen_t external_user_addrlen = sizeof(external_user_addr);
    external_user_fd = accept(service_proxy_fd, (struct sockaddr *) &external_user_addr, &external_user_addrlen);
    std::cout << "[Info] Accepted external connection: " << inet_ntoa(external_user_addr.sin_addr) << ':' << (int) ntohs(external_user_addr.sin_port) << '\n';

    //  generate uuid for this connection
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);

    //  send the uuid to client (client will use this id with a REDIRECT message to accept it)
    message.type = REDIRECT;
    message.string = std::string(uuid_str);
    std::unique_lock<std::mutex> user_id_lock(shared_resources::external_user_mutex);
    external_user_id.try_emplace(message.string, std::pair(external_user_fd, external_user_addr));
    user_id_lock.unlock();

    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  //  clean up
  close(service_proxy_fd);
  //  mark the port as available
  std::unique_lock<std::mutex> ports_close_lock(shared_resources::ports_mutex);
  proxy_ports_available.push(proxy_port);
  ports_close_lock.unlock();
}

void proxy_thread_func(SSL *client_ssl, int external_user_fd, std::atomic<bool> &flag_kill) {
  std::cout << "[Info] Proxying started\n";

  int ready_for_call = 0;
  ssize_t nbytes = 0;
  char buffer[32768];

  fd_set read_fd;
  timeval timev = {.tv_sec = 0, .tv_usec = 0};

  while (!flag_kill) {
    // client -> external_user
    ready_for_call = SSL_pending(client_ssl);
    if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = SSL_read(client_ssl, buffer, sizeof(buffer));

      if (nbytes <= 0) { std::cout << "[Info] Connection has been closed\n"; break; }
      if (send(external_user_fd, buffer, nbytes, 0) < 0) { std::cerr << "[Warning] Unable to send buffer to user (connection:proxy_thread)\n"; break; }
    }

    // external_user -> service
    FD_ZERO(&read_fd); FD_SET(external_user_fd, &read_fd);
    timev.tv_sec = 0; timev.tv_usec = 0;

    ready_for_call = select(external_user_fd + 1, &read_fd, nullptr, nullptr, &timev);
    if (ready_for_call < 0) {
      std::cerr << "[Warning] Invalid file descriptor passed to select (connection::proxy_thread)\n";
      break;
    } else if (ready_for_call > 0) {
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(external_user_fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) { std::cout << "[Info] Connection has been closed\n"; break; }
      if (SSL_write(client_ssl, buffer, nbytes) < 0) { std::cerr << "[Warning] Error occurred while sending buffer to client (connection:proxy_thread)\n"; break; }
    }
  }

  SSL_shutdown(client_ssl);
  SSL_free(client_ssl);

  close(external_user_fd);
  flag_kill = true;
  std::cout << "[Info] Proxying ended\n";
}