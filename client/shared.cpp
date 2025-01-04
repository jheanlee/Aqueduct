//
// Created by Jhean Lee on 2024/11/26.
//

#include "shared.hpp"

namespace shared_resources {
  std::mutex send_mutex;
  std::mutex ssl_send_mutex;
}