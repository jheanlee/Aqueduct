//
// Created by Jhean Lee on 2024/11/26.
//

#ifndef TUNNEL_SHARED_HPP
  #define TUNNEL_SHARED_HPP
  #include <mutex>
  #include <unordered_map>
  #include <atomic>
  #include <string>
  #include <queue>

  #include <sqlite3.h>

//  struct Client { //  TODO
//    std::atomic<bool> flag_kill;
//    const std::string ip_addr;
//    const std::string port;
//    const int type; //  CONNECT or REDIRECT
//    const int stream_port;  //  for CONNECT only, -1 for REDIRECT
//    std::atomic<int> external_connection_count;  //  for CONNECT only, -1 for REDIRECT
//    std::atomic<int> proxy_thread_count;  //  for CONNECT only, -1 for REDIRECT
//    std::atomic<unsigned long long> data_sent;
//    std::atomic<unsigned long long> data_recv;
//  };

  namespace shared_resources {
    extern std::mutex ssl_send_mutex;
    extern std::mutex ports_mutex;
    extern std::mutex external_user_mutex;
    extern sqlite3 *db;
    extern std::atomic<bool> global_flag_kill;
    extern std::atomic<bool> flag_handling_signal;
  }
  //  TODO: move these someday
  extern bool verbose;

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

  extern const char *db_path;
#endif //TUNNEL_SHARED_HPP
