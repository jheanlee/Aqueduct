#include "config.hpp"

std::vector<int> available_port(200);

void init_available_port() {
  std::iota(std::begin(available_port), std::end(available_port), available_port_begin);
}