//
// Created by Jhean Lee on 2024/10/2.
//
#include <thread>
#include <chrono>
#include <cstring>
#include <string>

#include <unistd.h>
#include <uuid/uuid.h>
#include <poll.h>

#include "socket_management.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "../common/shared.hpp"
#include "../database/database.hpp"
#include "../common/console.hpp"
#include "../database/client.hpp"

void ssl_control_thread_func() {
  int socket_fd = 0, status = 0;
  SSL_CTX *ctx;

  struct pollfd pfds[1];

  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(ssl_control_port)}, client_addr{};
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  //  create SSL context
  ctx = create_context();
  config_context (ctx);

  //  create, bind, listen socket
  socket_fd = create_socket(server_addr);

  console(NOTICE, CONNECTION_LISTEN_STARTED, nullptr, "connection::control");
  shared_resources::flag_tunneling_service_running = true;

  //  accept connections from client
  int client_fd = 0;
  socklen_t client_addrlen = sizeof(client_addr);

  pfds[0] = {.fd = socket_fd, .events = POLLIN | POLLPRI};
  while (!shared_resources::global_flag_kill) {
    status = poll(pfds, 1, timeout_session_millisec);

    if (status == 0) continue;
    else if (status < 0) {
      console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::control");
      continue;
    }
    client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addrlen);

    if (client_fd < 0) {
      console(ERROR, SOCK_ACCEPT_FAILED, nullptr, "connection::control");
      continue;
    }

    SSL *client_ssl;
    client_ssl = SSL_new(ctx);
    SSL_set_fd(client_ssl, client_fd);
    if (SSL_accept(client_ssl) <= 0) {
      console(ERROR, SSL_ACCEPT_FAILED, nullptr, "connection::control");
      continue;
    }

    console(INFO, CLIENT_CONNECTION_ACCEPTED, (std::string(inet_ntoa(client_addr.sin_addr)) + ':' + std::to_string((int)ntohs(client_addr.sin_port))).c_str(), "connection::control");
    std::thread session_thread(ssl_session_thread_func, client_fd, client_ssl, client_addr);
    session_thread.detach();
  }

  console(NOTICE, TUNNEL_SERVICE_ENDED, nullptr, "connection::control");
  shared_resources::flag_tunneling_service_running = false;

  //  clean up
  close(socket_fd);
  SSL_CTX_free(ctx);
}

void ssl_session_thread_func(int client_fd, SSL *client_ssl, sockaddr_in client_addr) {
  bool flag_first_msg = false, flag_connect_type = false, flag_proxy_type = false;
  std::atomic<bool> flag_heartbeat_received(false), flag_auth_received(false);
  int recv_status;
  char inbuffer[1024] = {0};
  std::string auth;
  std::mutex send_mutex;
  Message message {.type = '\0', .string = ""};
  Client *client;
  ClientData *client_data;

  struct pollfd pfds[1];

  std::thread heartbeat_thread; //  ensure connection is alive
  std::thread proxy_service_port_thread; // external_user <-> proxy_server connection, sends id to service(client)
  std::thread proxy_thread; //  external_user<->proxy_server <=== read/write i/o ===> proxy_server<->client(service)

  //  get flag_kill
  std::unique_lock<std::mutex> map_client_lock(shared_resources::map_client_mutex);
  shared_resources::map_flag_kill.try_emplace(shared_resources::map_key, false);
  std::atomic<bool> &flag_kill = shared_resources::map_flag_kill[shared_resources::map_key];
  size_t map_key = shared_resources::map_key;
  shared_resources::map_key++;
  map_client_lock.unlock();

  std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
  std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

  while (!shared_resources::global_flag_kill && !flag_kill && (flag_first_msg || duration <= std::chrono::seconds(first_message_timeout_sec))) {

    //  if client does not send message to server within <first_message_timeout_sec>, close the connection
    if (!flag_first_msg) duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);

    recv_status = ssl_read_message_non_block(client_ssl, pfds, inbuffer, sizeof(inbuffer), message);
    if (recv_status < 0) {  //  error or connection closed
      flag_kill = true;
    } else if (recv_status > 0) {
      switch (message.type) {
        case CONNECT:
          flag_first_msg = true; flag_connect_type = true;

          { //  new Client info
            map_client_lock.lock();
            std::pair<std::unordered_map<size_t, Client>::iterator, bool> iter_rtn = shared_resources::map_client.emplace(
                std::piecewise_construct, std::forward_as_tuple(map_key),
                std::forward_as_tuple(
                    map_key,
                    std::string(inet_ntoa(client_addr.sin_addr)),
                    (int) ntohs(client_addr.sin_port),
                    (CONNECT - 48)
                )
            );
            client = &(iter_rtn.first->second);
            map_client_lock.unlock();
          }

          //  ensure connection is alive
          heartbeat_thread = std::thread(ssl_heartbeat_thread_func, client_ssl, std::ref(client_addr), std::ref(flag_heartbeat_received),
                                         std::ref(flag_kill), std::ref(send_mutex));
          //  external_user <-> proxy_server connection, sends id to service(client)
          proxy_service_port_thread = std::thread(proxy_service_port_thread_func, std::ref(flag_kill),
                                                  std::ref(flag_auth_received), std::ref(auth), client_ssl, std::ref(client_addr),
                                                  std::ref(*client), std::ref(send_mutex));
          break;
        case HEARTBEAT:
          //  sets heartbeat flag to true (used by heartbeat_thread)
          flag_heartbeat_received = true;
          break;
        case REDIRECT:
          flag_first_msg = true;
          {
            //  find external_user (fd and extra info)
            std::unique_lock<std::mutex> external_lock(shared_resources::external_user_mutex);
            if (shared_resources::external_user_id_map.find(message.string) != shared_resources::external_user_id_map.end()) {
              //  open proxy_thread for client-accepted external_user (by id)
              External_User external_user = shared_resources::external_user_id_map[message.string];
              shared_resources::external_user_id_map.erase(message.string);

              //  new Client info
              map_client_lock.lock();
              std::pair<std::unordered_map<size_t, Client>::iterator, bool> iter_rtn = shared_resources::map_client.emplace(
                  std::piecewise_construct, std::forward_as_tuple(map_key),
                  std::forward_as_tuple(
                      map_key,
                      std::string(inet_ntoa(client_addr.sin_addr)),
                      (int) ntohs(client_addr.sin_port),
                      (REDIRECT - 48),
                      std::string(inet_ntoa(external_user.external_user.sin_addr)),
                      (int) ntohs(external_user.external_user.sin_port),
                      std::string(inet_ntoa(external_user.client.sin_addr)),
                      (int) ntohs(external_user.client.sin_port)
                  )
              );

              std::pair<std::unordered_map<size_t, ClientData>::iterator, bool> iter_data_rtn = shared_resources::map_client_data.emplace(
                  std::piecewise_construct, std::forward_as_tuple(map_key),
                  std::forward_as_tuple(
                      std::string(inet_ntoa(external_user.client.sin_addr))
                  )
              );
              client = &(iter_rtn.first->second);
              client_data = &(iter_data_rtn.first->second);
              map_client_lock.unlock();

              //  external_user<->proxy_server <=== read/write io ===> proxy_server<->client(service)
              proxy_thread = std::thread(proxy_thread_func, client_ssl, external_user, std::ref(flag_kill), std::ref(*client), std::ref(*client_data));
              flag_proxy_type = true;
            } else flag_kill = true;
            external_lock.unlock();
          }
          goto proxy;
        case AUTHENTICATION:
          flag_auth_received = true;
          auth = message.string;
          break;
        default:
          flag_kill = true;
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

  map_client_lock.lock();
  shared_resources::map_flag_kill.erase(map_key); //  At this point, other threads holding the reference to this would have ended
  if (flag_proxy_type) update_client_db(shared_resources::map_client_data.find(map_key)->second);
  shared_resources::map_client.erase(map_key);
  map_client_lock.unlock();

  console(DEBUG, CONNECTION_CLOSED, (std::string(inet_ntoa(client_addr.sin_addr)) + ':' + std::to_string((int)ntohs(client_addr.sin_port))).c_str(), "connection::sesstion");
}

void ssl_heartbeat_thread_func(SSL *client_ssl, sockaddr_in &client_addr, std::atomic<bool> &flag_heartbeat_received, std::atomic<bool> &flag_kill, std::mutex &send_mutex) {
  char outbuffer[1024] = {0};
  Message heartbeat_message = {.type = HEARTBEAT, .string = ""};

  std::chrono::system_clock::time_point timer;
  std::chrono::seconds heartbeat_duration;

  while (!flag_kill) {
    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
    //  send heartbeat message

    if (ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), heartbeat_message, send_mutex) <= 0) {
      console(WARNING, MESSAGE_SEND_FAILED, nullptr, "connection::heartbeat");
    }

    //  start timing
    timer = std::chrono::system_clock::now();
    flag_heartbeat_received = false;

    //  wait for heartbeat
    while (!flag_kill && !flag_heartbeat_received) {
      heartbeat_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);

      if (heartbeat_duration > std::chrono::seconds(heartbeat_timeout_sec)) {
        console(INFO, HEARTBEAT_TIMEOUT, (std::string(inet_ntoa(client_addr.sin_addr)) + ':' + std::to_string((int)ntohs(client_addr.sin_port))).c_str(), "connection::heartbeat");
        flag_kill = true;
        return;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

void proxy_service_port_thread_func(std::atomic<bool> &flag_kill, std::atomic<bool> &flag_auth_received, std::string &auth, SSL *client_ssl, sockaddr_in &client_addr, Client &client, std::mutex &send_mutex) {
  bool flag_port_found = false;
  int service_proxy_fd = 0, proxy_port = 0, external_user_fd = 0, status = 0;
  char outbuffer[1024] = {0}, uuid_str[37] = {0};

  struct pollfd pfds[1];

  struct sockaddr_in server_proxy_addr = {.sin_family = AF_INET}, external_user_addr = {.sin_family = AF_INET};
  inet_pton(AF_INET, host, &server_proxy_addr.sin_addr);

  uuid_t uuid;
  Message message = {.type = AUTHENTICATION, .string = ""};

  //  authentication
  ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);

  while (!flag_kill && !flag_auth_received) std::this_thread::yield();
  if (auth.empty()) {
    message = {.type = AUTH_FAILED, .string = ""};
    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
    console(NOTICE, AUTHENTICATION_FAILED, (std::string(inet_ntoa(client_addr.sin_addr)) + ':' + std::to_string((int)ntohs(client_addr.sin_port))).c_str(), "connection::proxy_service");
    flag_kill = true;
    return;
  }

  const char sql[256] = "SELECT EXISTS("
                        "SELECT auth.token FROM auth "
                        "WHERE auth.token = base32_encode(sha256(? || ?))"
                        ") AS sql_result;";
  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    message = {.type = DB_ERROR, .string = ""};
    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "connection::proxy_service");
    flag_kill = true;
    return;
  }

  if (sqlite3_bind_text(stmt, 1, auth.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK  ||
      sqlite3_bind_text(stmt, 2, shared_resources::db_salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)  {
    message = {.type = DB_ERROR, .string = ""};
    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "connection::proxy_service");
    sqlite3_finalize(stmt);
    flag_kill = true;
    return;
  }

  int sql_result = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    sql_result = sqlite3_column_int(stmt, 0);
  } else {
    message = {.type = DB_ERROR, .string = ""};
    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
    console(ERROR, SQLITE_RETRIEVE_FAILED, sqlite3_errmsg(shared_resources::db), "connection::proxy_service");
    flag_kill = true;
    return;
  }
  sqlite3_finalize(stmt);

  if (sql_result == 1) {
    message = {.type = AUTH_SUCCESS, .string = ""};
    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
    console(NOTICE, AUTHENTICATION_SUCCESS, (std::string(inet_ntoa(client_addr.sin_addr)) + ':' + std::to_string((int)ntohs(client_addr.sin_port))).c_str(), "connection::proxy_service");
  } else {
    message = {.type = AUTH_FAILED, .string = ""};
    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
    console(NOTICE, AUTHENTICATION_FAILED, (std::string(inet_ntoa(client_addr.sin_addr)) + ':' + std::to_string((int)ntohs(client_addr.sin_port))).c_str(), "connection::proxy_service");
    flag_kill = true;
    return;
  }

  message = {.type = STREAM_PORT, .string = ""};

  //  create socket
  service_proxy_fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tcp
  if (service_proxy_fd == -1) {
    console(ERROR, SOCK_CREATE_FAILED, nullptr, "connection::proxy_service");
    flag_kill = true;
    return;
  }

  //  set proxy port
  std::unique_lock<std::mutex> ports_assign_lock(shared_resources::ports_mutex);
  while (!flag_port_found && !proxy_ports_available.empty()) {
    server_proxy_addr.sin_port = htons(proxy_ports_available.front());

    if (bind_socket(service_proxy_fd, server_proxy_addr) != 0) { proxy_ports_available.pop(); continue; }

    console(NOTICE, PROXY_PORT_NEW, (std::string(host) + ':' + std::to_string(proxy_ports_available.front())).c_str(), "connection::proxy_service");
    proxy_port = proxy_ports_available.front();
    proxy_ports_available.pop();
    flag_port_found = true;
  }
  if (proxy_ports_available.empty() && !flag_port_found) {
    console(WARNING, NO_PORT_AVAILABLE, nullptr, "connection::proxy_service");
    message.type = NO_PORT;
    flag_kill = true;
    ports_assign_lock.unlock();
    return;
  }
  ports_assign_lock.unlock();

  //  send the port to which the service is streamed to client (external users can connect to this port)
  message.string = std::to_string((int) ntohs(server_proxy_addr.sin_port));
  client.stream_port = (int) ntohs(server_proxy_addr.sin_port);
  ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);

  //  accept external connections (end users)
  pfds[0] = {.fd = service_proxy_fd, .events = POLLIN | POLLPRI};
  while (!flag_kill) {
    status = poll(pfds, 1, timeout_proxy_millisec);
    if (status == 0) continue;
    else if (status < 0) {
      console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy_service");
      close(service_proxy_fd);
      flag_kill = true;
      return;
    }

    socklen_t external_user_addrlen = sizeof(external_user_addr);
    external_user_fd = accept(service_proxy_fd, (struct sockaddr *) &external_user_addr, &external_user_addrlen);
    console(INFO, EXTERNAL_CONNECTION_ACCEPTED, (std::string(inet_ntoa(external_user_addr.sin_addr)) + ':' + std::to_string((int)ntohs(external_user_addr.sin_port))).c_str(), "connection::proxy_service");

    //  generate uuid for this connection
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);

    //  send the uuid to client (client will use this id with a REDIRECT message to accept it)
    message.type = REDIRECT;
    message.string = std::string(uuid_str);
    std::unique_lock<std::mutex> user_id_lock(shared_resources::external_user_mutex);
    shared_resources::external_user_id_map.try_emplace(message.string, External_User{ .fd = external_user_fd, .external_user = external_user_addr, .client = client_addr, .server = server_proxy_addr });
    user_id_lock.unlock();

    ssl_send_message(client_ssl, outbuffer, sizeof(outbuffer), message, send_mutex);
  }

  //  clean up
  close(service_proxy_fd);
  //  mark the port as available
  std::unique_lock<std::mutex> ports_close_lock(shared_resources::ports_mutex);
  if (proxy_port != 0) proxy_ports_available.push(proxy_port);
  ports_close_lock.unlock();
}

void proxy_thread_func(SSL *client_ssl, External_User external_user, std::atomic<bool> &flag_kill, Client &client, ClientData &client_data) {
  console(DEBUG, PROXYING_STARTED, (client.ip_addr + ':' + std::to_string(client.port) + " <=> " + std::string(inet_ntoa(external_user.external_user.sin_addr)) + ':' + std::to_string((int)ntohs(external_user.external_user.sin_port))).c_str(), "connection::proxy");

  int ready_for_call = 0, ready_for_write = 0, write_status = 0;
  ssize_t nbytes = 0, io = 0;
  char buffer[32768];

  struct pollfd pfds[1];

  while (!flag_kill) {
    //  client -> external_user
    pfds[0] = {.fd = SSL_get_fd(client_ssl), .events = POLLIN | POLLPRI};
    ready_for_call = poll(pfds, 1, timeout_proxy_millisec);
    if (ready_for_call < 0) {
      console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
      break;
    } else if (ready_for_call > 0) {
      //  read from client
      memset(buffer, 0, sizeof(buffer));
      nbytes = SSL_read(client_ssl, buffer, sizeof(buffer));
      if (nbytes <= 0) {
        console(DEBUG, CONNECTION_CLOSED_BY_CLIENT, (client.ip_addr + ':' + std::to_string(client.port)).c_str(), "connection::proxy");
        break;
      }

      //  send to external_user
      pfds[0] = {.fd = external_user.fd, .events = POLLOUT | POLLWRBAND};
      ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      while (!flag_kill && ready_for_write == 0) {
        ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      }
      if (ready_for_write < 0) {
        console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
        break;
      }
      io = send(external_user.fd, buffer, nbytes, MSG_NOSIGNAL);
      if (io < 0) {
        if (errno == EPIPE) {
          console(DEBUG, CONNECTION_CLOSED_BY_EXTERNAL_USER, (std::string(inet_ntoa(external_user.external_user.sin_addr)) + ':' + std::to_string((int)ntohs(external_user.external_user.sin_port))).c_str(), "connection::proxy");
        } else {
          console(ERROR, BUFFER_SEND_ERROR_TO_CLIENT, (client.ip_addr + ':' + std::to_string(client.port) + " => " + std::string(inet_ntoa(external_user.external_user.sin_addr)) + ':' + std::to_string((int)ntohs(external_user.external_user.sin_port))).c_str(), "connection::proxy");
        }
        break;
      } else {
        client_data.data_sent += io;
      }
    }

    //  external_user -> client
    pfds[0] = {.fd = external_user.fd, .events = POLLIN | POLLPRI};
    ready_for_call = poll(pfds, 1, timeout_proxy_millisec);
    if (ready_for_call < 0) {
      console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
      break;
    } else if (ready_for_call > 0) {
      //  read from external user
      memset(buffer, 0, sizeof(buffer));
      nbytes = recv(external_user.fd, buffer, sizeof(buffer), 0);
      if (nbytes <= 0) {
        console(DEBUG, CONNECTION_CLOSED_BY_EXTERNAL_USER, (std::string(inet_ntoa(external_user.external_user.sin_addr)) + ':' + std::to_string((int)ntohs(external_user.external_user.sin_port))).c_str(), "connection::proxy");
        break;
      }

      //  send to client
      pfds[0] = {.fd = SSL_get_fd(client_ssl), .events = POLLOUT | POLLWRBAND};
      ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      while (!flag_kill && ready_for_write == 0) {
        ready_for_write = poll(pfds, 1, timeout_proxy_millisec);
      }
      if (ready_for_write < 0) {
        console(ERROR, SOCK_POLL_ERR, std::to_string(errno).c_str(), "connection::proxy");
        break;
      }

      write_status = SSL_write(client_ssl, buffer, nbytes);
      if (write_status < 0) {
        if (write_status == -1 && SSL_get_error(client_ssl, write_status)) {
          console(DEBUG, CONNECTION_CLOSED_BY_CLIENT, (client.ip_addr + ':' + std::to_string(client.port)).c_str(), "connection::proxy");
        } else {
          console(ERROR, BUFFER_SEND_ERROR_TO_CLIENT, (client.ip_addr + ':' + std::to_string(client.port) + " <= " + std::string(inet_ntoa(external_user.external_user.sin_addr)) + ':' + std::to_string((int)ntohs(external_user.external_user.sin_port))).c_str(), "connection::proxy");
        }
        break;
      } else if (write_status == 0) {
        console(DEBUG, CONNECTION_CLOSED_BY_CLIENT, (client.ip_addr + ':' + std::to_string(client.port)).c_str(), "connection::proxy");
        break;
      } else {
        client_data.data_recv += write_status;
      }
    }
  }

  close(external_user.fd);
  flag_kill = true;

  console(DEBUG, PROXYING_ENDED, (client.ip_addr + ':' + std::to_string(client.port) + " <=> " + std::string(inet_ntoa(external_user.external_user.sin_addr)) + ':' + std::to_string((int)ntohs(external_user.external_user.sin_port))).c_str(), "connection::proxy");
}