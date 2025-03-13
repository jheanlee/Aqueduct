//
// Created by Jhean Lee on 2024/11/26.
//

#ifndef SPHERE_LINKED_SHARED_HPP
  #define SPHERE_LINKED_SHARED_HPP
  #include <mutex>
  #include <unordered_map>
  #include <atomic>
  #include <string>
  #include <queue>
  #include <utility>
  #include <chrono>
  #include <cstdio>

  #include <sqlite3.h>
  #include <netinet/in.h>

  struct Client {
    const size_t key;
    const std::string ip_addr;
    const int port;
    const int type;                       //  CONNECT or REDIRECT
    int stream_port = -1;                 //  for CONNECT only, -1 for REDIRECT
    const std::string user_addr;          //  for REDIRECT only, empty for CONNECT
    const int user_port;                  //  for REDIRECT only, -1 for CONNECT
    const std::string main_ip_addr;       //  the CONNECT client of this client; for REDIRECT only, empty for CONNECT
    const int main_port;                  //  the CONNECT client of this client; for REDIRECT only, -1 for CONNECT

    Client(size_t key_c, std::string ip_addr_c, int port_c, int type_c, std::string user_addr_c = "", int user_port_c = -1, std::string main_ip_addr_c = "", int main_port_c = -1)
      : key(key_c), ip_addr(std::move(ip_addr_c)), port(port_c), type(type_c), user_addr(std::move(user_addr_c)), user_port(user_port_c), main_ip_addr(std::move(main_ip_addr_c)), main_port(main_port_c){}
  };
  struct ClientData{
    const std::string main_ip_addr; //  empty for CONNECT
    std::atomic<size_t> data_sent = 0;
    std::atomic<size_t> data_recv = 0;

    explicit ClientData(std::string main_ip_addr_c = "") : main_ip_addr(std::move(main_ip_addr_c)) {}
  };

  struct External_User {
    int fd;
    sockaddr_in external_user;
    sockaddr_in client;
    sockaddr_in server;                   //  the proxy port this user is connected to
  };

  namespace shared_resources {
    extern std::atomic<bool> global_flag_kill;
    extern std::atomic<bool> flag_handling_signal;
    extern std::atomic<bool> flag_tunneling_service_running;
    extern std::atomic<bool> flag_api_service_running;
    extern std::atomic<bool> flag_api_kill;

    extern std::chrono::time_point<std::chrono::system_clock> process_start;

    extern std::mutex cout_mutex;
    extern std::mutex ports_mutex;

    extern sqlite3 *db;
    extern std::string db_salt;
    extern int client_db_interval_min;

    extern std::atomic<size_t> map_key;
    extern std::unordered_map<size_t, std::atomic<bool>> map_flag_kill;
    extern std::unordered_map<size_t, Client> map_client;
    extern std::unordered_map<size_t, ClientData> map_client_data;
    extern std::mutex map_client_mutex;
    extern std::unordered_map<size_t, Client> map_client_copy;
    extern std::mutex map_client_copy_mutex;

    extern std::unordered_map<std::string, External_User> external_user_id_map; // id, {external_user_fd, addr}
    extern std::mutex external_user_mutex;

    extern pid_t pid_api;
  }
  //  TODO: move these someday
  extern int verbose;

  extern const char *cert_path;
  extern const char *key_path;

  static const char *host = "0.0.0.0";
  extern int ssl_control_port;
  static const int connection_limit = 5;

  extern int proxy_port_start;
  extern int proxy_port_limit;
  extern std::queue<int> proxy_ports_available;

  static const int first_message_timeout_sec = 30;
  static const int heartbeat_sleep_sec = 30;
  static const int heartbeat_timeout_sec = 30;

  extern int timeout_session_millisec;
  extern int timeout_proxy_millisec;
  extern int timeout_api_millisec;

  extern const char *db_path;
#endif //SPHERE_LINKED_SHARED_HPP
