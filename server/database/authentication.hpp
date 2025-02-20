//
// Created by Jhean Lee on 2025/2/6.
//

#ifndef SPHERE_LINKED_AUTHENTICATION_HPP
  #define SPHERE_LINKED_AUTHENTICATION_HPP
  #include <string>

  int new_token(const std::string &name, const std::string &notes, int expiry_days);
  int remove_token(const std::string &name);
  int list_token();
  int check_token_expiry();

#endif //SPHERE_LINKED_AUTHENTICATION_HPP
