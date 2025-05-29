//
// Created by Jhean Lee on 2025/2/14.
//

#include "client.hpp"
#include "../common/console.hpp"

#include <chrono>
#include <mutex>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <unordered_map>

static const char *update_client = "INSERT INTO client(ip, sent, received) "
                                   "VALUES (?, ?, ?) "
                                   "ON CONFLICT(ip) DO UPDATE "
                                   "SET sent = sent + ?, "
                                   "received = received + ?;";

int update_client_db(ClientData &client) {
  if (client.main_ip_addr.empty()) return 1;
  sqlite3_stmt *stmt = nullptr;
  size_t sent = client.data_sent.fetch_add(0), recv = client.data_recv.fetch_add(0);
  client.data_sent = 0; client.data_recv = 0;

  if (sqlite3_prepare_v2(shared_resources::db, update_client, -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "client::update_client_db");
    return -1;
  }

  if (sqlite3_bind_text(stmt, 1, client.main_ip_addr.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_int64(stmt, 2, sent) != SQLITE_OK ||
      sqlite3_bind_int64(stmt, 3, recv) != SQLITE_OK ||
      sqlite3_bind_int64(stmt, 4, sent) != SQLITE_OK ||
      sqlite3_bind_int64(stmt, 5, recv) != SQLITE_OK) {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "client::update_client_db");
    sqlite3_finalize(stmt);
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(ERROR, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "client::update_client_db");
    sqlite3_finalize(stmt);
    return -1;
  }

  sqlite3_finalize(stmt);
  return 0;
}

void update_client_db_thread_func() {
  std::unique_lock<std::mutex> client_lock(shared_resources::map_client_mutex, std::defer_lock);

  std::chrono::system_clock::time_point last_updated = std::chrono::system_clock::now();
  std::chrono::minutes duration;

  while (!shared_resources::global_flag_kill) {
    //  timeout
    duration = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - last_updated);
    while (!shared_resources::global_flag_kill && duration < std::chrono::minutes(config::client_db_interval_min)) {
      std::this_thread::sleep_for(std::chrono::seconds(10));
      duration = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - last_updated);
    }
    last_updated = std::chrono::system_clock::now();

    //  iterate through map and set all Clients
    client_lock.lock();
    if (shared_resources::db) {
      for (std::pair<const size_t, ClientData> &client_data: shared_resources::map_client_data) {
        update_client_db(client_data.second);
      }
    } else {
      console(ERROR, INVALID_DB, nullptr, "client::update_client_db");
    }
    client_lock.unlock();
  }
}

void update_client_copy() {
  std::unique_lock<std::mutex> client_lock(shared_resources::map_client_mutex, std::defer_lock);
  std::unique_lock<std::mutex> client_copy_lock(shared_resources::map_client_copy_mutex, std::defer_lock);

  client_lock.lock();
  client_copy_lock.lock();
  shared_resources::map_client_copy = std::unordered_map<size_t, Client> (shared_resources::map_client);
  client_copy_lock.unlock();
  client_lock.unlock();
}

template <typename T>
static void print_element(T value, int width) {
  std::stringstream buffer;
  buffer << std::left << std::setw(width) << std::setfill(' ') << value;
  std::cout << buffer.str();
}

void list_clients() {
  std::unique_lock<std::mutex> client_copy_lock(shared_resources::map_client_copy_mutex);

  std::cout << '\n';
  print_element("Key", 8);
  print_element("Client_IP", 25);
  print_element("Client_Type", 15);
  print_element("Stream_Port", 15);
  print_element("User_IP(Proxy)", 25);
  print_element("Client Main IP", 25);
  std::cout << '\n';
  for (std::pair<const size_t, Client> &client: shared_resources::map_client_copy) {
    print_element(client.second.key, 8);
    print_element(client.second.ip_addr + ':' + std::to_string(client.second.port), 25);
    print_element(client.second.type, 15);
    print_element(client.second.stream_port, 15);
    print_element(client.second.user_addr + ':' + std::to_string(client.second.user_port), 25);
    print_element(client.second.main_ip_addr + ':' + std::to_string(client.second.main_port), 25);
    std::cout << '\n';
  }
  std::cout << '\n' << std::flush;
  client_copy_lock.unlock();
}