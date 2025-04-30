//
// Created by Jhean Lee on 2025/3/4.
//

#include <cstring>
#include <thread>
#include <string>
#include <vector>

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

#include "api.hpp"
#include "../common/shared.hpp"
#include "../common/console.hpp"
#include "../tunnel/message.hpp"
#include "../database/client.hpp"
#include "token.hpp"

static const char *socket_path = "/tmp/aqueduct-server-core.sock";

void api_control_thread_func() {
  std::vector<std::thread> api_threads;

  int core_fd = socket(AF_UNIX, SOCK_STREAM, 0), api_fd, status; // unix domain socket, tcp
  sockaddr_un core_addr{}, api_addr{.sun_family = AF_UNIX};
  core_addr.sun_family = AF_UNIX;
  strcpy(core_addr.sun_path, socket_path);

  socklen_t core_addrlen = sizeof(core_addr);
  socklen_t api_addrlen = sizeof(struct sockaddr_un);
  struct pollfd pfds[1];

  unlink(socket_path);

  if (core_fd == -1) {
    console(ERROR, API_SOCK_CREATE_FAILED, nullptr, "api::api_control");
    shared_resources::flag_api_kill = true;
    return;
  }
  if (bind(core_fd, (struct sockaddr *) &core_addr, core_addrlen) == -1) {
    console(ERROR, API_SOCK_BIND_FAILED, nullptr, "api::api_control");
    shared_resources::flag_api_kill = true;
    return;
  }
  if (listen(core_fd, connection_limit) == -1) {
    console(ERROR, API_SOCK_LISTEN_FAILED, nullptr, "api::api_control");
    shared_resources::flag_api_kill = true;
    return;
  }

  console(INFO, API_LISTEN_STARTED, nullptr, "api::api_control");
  shared_resources::flag_api_service_running = true;

  pfds[0] = {.fd = core_fd, .events = POLLIN | POLLPRI};
  while (!shared_resources::global_flag_kill && !shared_resources::flag_api_kill) {
    status = poll(pfds, 1, timeout_api_millisec);

    if (status == 0) continue;
    else if (status < 0) {
      console(ERROR, API_SOCK_POLL_ERR, std::to_string(errno).c_str(), "api::api_control");
      continue;
    }

    api_fd = accept(core_fd, (struct sockaddr *) &api_addr, &api_addrlen);
    if (api_fd < 0) {
      console(ERROR, API_SOCK_ACCEPT_FAILED, nullptr, "api::api_control");
      continue;
    }

    api_threads.emplace_back(api_session_thread_func, api_fd, api_addr);
  }

  console(INFO, API_SERVICE_ENDED, nullptr, "api::api_control");
  shared_resources::flag_api_service_running = false;
  shared_resources::flag_api_kill = true;

  for (std::thread &t : api_threads) {
    t.join();
  }
}

void api_session_thread_func(int api_fd, sockaddr_un api_addr) {
  console(INFO, API_CLIENT_CONNECTION_ACCEPTED, nullptr, "api::api_session");

  std::atomic<bool> flag_kill(false), flag_heartbeat_received(false);
  char inbuffer[256] = {0}, outbuffer[256] = {0}, client_buffer[32768];
  int recv_status;
  std::mutex send_mutex;
  std::thread heartbeat_thread(api_heartbeat_thread_func, std::ref(flag_kill), std::ref(api_fd), std::ref(send_mutex), std::ref(flag_heartbeat_received));

  Message message = {.type = '\0', .string = ""};

  struct pollfd pfds[1];

  while (!shared_resources::global_flag_kill && !shared_resources::flag_api_kill && !flag_kill) {
    recv_status = read_message_non_block(api_fd, pfds, inbuffer, sizeof(inbuffer), message);

    if (recv_status < 0) {
      flag_kill = true;
    } else if (recv_status > 0) {
      switch (message.type) {
        case API_HEARTBEAT:
          flag_heartbeat_received = true;
          break;
        case API_EXIT:
          flag_kill = true;
          console(INFO, API_CONNECTION_CLOSED, nullptr, "api::api_session");
          break;
        case API_GET_SERVICE_INFO: {
          std::chrono::duration uptime_duration = std::chrono::system_clock::now() - shared_resources::process_start;
          nlohmann::json service_info_json;
          service_info_json["uptime"] = std::chrono::duration_cast<std::chrono::seconds>(uptime_duration).count();
          service_info_json["tunnel_service_up"] = shared_resources::flag_tunneling_service_running.load();
          service_info_json["api_service_up"] = shared_resources::flag_api_service_running.load();
          service_info_json["connected_clients"] = shared_resources::map_client.size();
          message.type = API_GET_SERVICE_INFO;
          message.string = to_string(service_info_json);
          send_message(api_fd, outbuffer, sizeof(outbuffer), message, send_mutex);
          break;
        }
        case API_GET_CURRENT_CLIENTS: {
          nlohmann::json clients_json;
          update_client_copy();

          clients_json["clients"] = nlohmann::json::array();
          std::lock_guard<std::mutex> lock(shared_resources::map_client_copy_mutex);
          for (auto &client : shared_resources::map_client_copy) {
            nlohmann::json client_json;
            client_json["key"] = client.second.key;
            client_json["ip_addr"] = client.second.ip_addr;
            client_json["port"] = client.second.port;
            client_json["type"] = client.second.type;
            client_json["stream_port"] = client.second.stream_port;
            client_json["user_addr"] = client.second.user_addr;
            client_json["user_port"] = client.second.user_port;
            client_json["main_addr"] = client.second.main_ip_addr;
            client_json["main_port"] = client.second.main_port;
            clients_json["clients"].push_back(client_json);
          }
          message.type = API_GET_CURRENT_CLIENTS;
          message.string = to_string(clients_json);
          send_large_message(api_fd, client_buffer, sizeof(client_buffer), message, send_mutex);  //  TODO: split if too many clients
          break;
        }
        case API_GENERATE_NEW_TOKEN: {
          std::pair<std::string, std::string> new_token = generate_token();

          nlohmann::json token_json;
          token_json["token"] = new_token.first;
          token_json["hashed"] = new_token.second;
          message.type = API_GENERATE_NEW_TOKEN;
          message.string = to_string(token_json);
          send_message(api_fd, outbuffer, sizeof(outbuffer), message, send_mutex);
          break;
        }
        default:
          flag_kill = true;
          break;
      }
    }
  }

  close(api_fd);

  flag_kill = true;

  heartbeat_thread.join();
}

void api_heartbeat_thread_func(std::atomic<bool> &flag_kill, int &api_fd, std::mutex &send_mutex, std::atomic<bool> &flag_heartbeat_received) {
  std::unique_lock<std::mutex> lock(send_mutex, std::defer_lock);

  char outbuffer[256] = {0};
  Message heartbeat_message = {.type = API_HEARTBEAT, .string = ""};

  std::chrono::system_clock::time_point timer;
  std::chrono::seconds heartbeat_duration;

  while (!flag_kill) {
    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));

    //  send heartbeat message
    if (send_message(api_fd, outbuffer, sizeof(outbuffer), heartbeat_message, send_mutex) <= 0) {
      console(WARNING, MESSAGE_SEND_FAILED, nullptr, "api::api_heartbeat");
    }

    //  start timing
    timer = std::chrono::system_clock::now();
    flag_heartbeat_received = false;

    //  wait for heartbeat
    while (!flag_kill && !flag_heartbeat_received) {
      heartbeat_duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - timer);

      if (heartbeat_duration > std::chrono::seconds(heartbeat_timeout_sec)) {
        console(INFO, API_HEARTBEAT_TIMEOUT, nullptr, "api::api_heartbeat");
        flag_kill = true;
        return;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}