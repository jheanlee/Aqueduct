//
// Created by Jhean Lee on 2024/7/27.
//

#include "random_key.hpp"

int random(int lower_bound, int upper_bound) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(lower_bound, upper_bound);
  return dis(gen);
}

std::string random_key_gen() {
  std::string new_key;

  for (int i = 0; i < 32; i++) {
    new_key.push_back(key_chars[random(0, 61)]);
  }

  return new_key;
}
