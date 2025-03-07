//
// Created by Jhean Lee on 2024/11/26.
//

#ifndef SPHERE_LINKED_SHARED_HPP
  #define SPHERE_LINKED_SHARED_HPP
  #include <mutex>

  namespace shared_resources {
    extern std::mutex ssl_send_mutex;
  }

  extern bool verbose;

#endif //SPHERE_LINKED_SHARED_HPP
