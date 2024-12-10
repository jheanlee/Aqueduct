//
// Created by Jhean Lee on 2024/11/26.
//

#ifndef TUNNEL_SHARED_HPP
  #define TUNNEL_SHARED_HPP
  #include <mutex>

  namespace shared_resources {
    extern std::mutex send_mutex;
    extern std::mutex ports_mutex;
  }

#endif //TUNNEL_SHARED_HPP
