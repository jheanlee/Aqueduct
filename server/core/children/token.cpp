//
// Created by Jhean Lee on 2025/4/15.
//

#include <openssl/evp.h>

#include "../database/database.hpp"
#include "../common/shared.hpp"
#include "../common/console.hpp"

#include "token.hpp"

std::pair<std::string, std::string> generate_token() {
  std::string buffer;
  if (generate_salt(buffer, 32) < 0) return {"", ""};
  std::string token = "AQ_" + buffer;


  unsigned char sha_buffer[EVP_MAX_MD_SIZE];
  sha256(reinterpret_cast<const unsigned char *>((token + shared_resources::db_salt).c_str()), sha_buffer);

  unsigned char base32_buffer[56];
  encode_base32(sha_buffer, 32, base32_buffer, 56);

  buffer = std::string(reinterpret_cast<char*>(base32_buffer), 56);

  return {token, buffer};
}
