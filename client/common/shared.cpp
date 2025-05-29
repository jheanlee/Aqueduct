//
// Created by Jhean Lee on 2024/11/26.
//

#include "shared.hpp"

namespace config {
  bool daemon_mode = false;
}

namespace shared_resources {
  #if defined(__OS_LOG_H__)
    os_log_t os_log_aqueduct;
  #endif

  std::mutex cout_mutex;
}