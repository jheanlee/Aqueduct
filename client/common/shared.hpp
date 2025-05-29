//
// Created by Jhean Lee on 2024/11/26.
//

#ifndef AQUEDUCT_SHARED_HPP
  #define AQUEDUCT_SHARED_HPP
  #include <mutex>

  #if defined(__clang__) && defined(__APPLE__)
    #include <os/log.h>
  #endif

  namespace config {
    extern bool daemon_mode;
  }

  namespace shared_resources {
    #if defined(__OS_LOG_H__)
      extern os_log_t os_log_aqueduct;
    #endif

    extern std::mutex cout_mutex;
  }

  extern int verbose_level;

#endif //AQUEDUCT_SHARED_HPP
