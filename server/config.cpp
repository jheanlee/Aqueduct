//
// Created by Jhean Lee on 2024/10/2.
//

#include "config.hpp"

std::queue<int> proxy_ports_available;
void init_proxy_ports_available() {
  for (int i = proxy_port_start; i < proxy_port_start + proxy_port_limit; i++) {
      proxy_ports_available.push(i);
  }
}