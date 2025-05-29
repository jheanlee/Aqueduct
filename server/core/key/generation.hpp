//
// Created by Jhean Lee on 2025/5/1.
//

#ifndef AQUEDUCT_GENERATION_HPP
  #define AQUEDUCT_GENERATION_HPP
  #include <string>

  int generate_ssl_key_cert(const std::string &root_path);
  int generate_jwt_key_pair(const std::string &root_path, const std::string &prefix);

#endif //AQUEDUCT_GENERATION_HPP
