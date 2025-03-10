//
// Created by Jhean Lee on 2024/11/26.
//

#include "shared.hpp"

namespace shared_resources {
  std::atomic<bool> global_flag_kill(false);
  std::atomic<bool> flag_handling_signal(false);
  std::atomic<bool> flag_tunneling_service_running(false);
  std::atomic<bool> flag_api_service_running(false);
  std::atomic<bool> flag_api_kill(false);

  std::chrono::time_point<std::chrono::system_clock> process_start;

  std::mutex cout_mutex;
  std::mutex ports_mutex;

  sqlite3 *db = nullptr;
  std::string db_salt;

  std::atomic<size_t> map_key = 0;
  std::unordered_map<size_t, std::atomic<bool>> map_flag_kill;
  std::unordered_map<size_t, Client> map_client;
  std::mutex map_client_mutex;

  std::unordered_map<std::string, External_User> external_user_id_map;
  std::mutex external_user_mutex;

  pid_t pid_api = 0;
}
std::queue<int> proxy_ports_available;

