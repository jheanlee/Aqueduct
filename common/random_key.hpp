//
// Created by Jhean Lee on 2024/7/27.
//

#ifndef TUNNEL_RANDOM_KEY_HPP
  #define TUNNEL_RANDOM_KEY_HPP
  #include <random>
  #include <string>

static const char* key_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int random(int lower_bound, int upper_bound);

std::string random_key_gen();

#endif //TUNNEL_RANDOM_KEY_HPP
