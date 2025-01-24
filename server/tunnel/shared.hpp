//
// Created by Jhean Lee on 2024/11/26.
//

#ifndef TUNNEL_SHARED_HPP
  #define TUNNEL_SHARED_HPP
  #include <mutex>
  #include <unordered_map>
  #include <atomic>
  #include <string>

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
  }

#endif //TUNNEL_SHARED_HPP
