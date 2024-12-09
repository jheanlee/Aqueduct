//
// Created by Jhean Lee on 2024/10/2.
//
#include <numeric>

#include "config.hpp"

std::vector<int> proxy_port_available(200);
void init_proxy_port_available() {
  std::iota(std::begin(proxy_port_available), std::end(proxy_port_available), proxy_port_begin);
}