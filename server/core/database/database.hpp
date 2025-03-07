//
// Created by Jhean Lee on 2025/1/21.
//

#ifndef SPHERE_LINKED_DATABASE_HPP
  #define SPHERE_LINKED_DATABASE_HPP
  #include <sqlite3.h>
  #include <string>

  void open_db(sqlite3 **db);
  void create_sqlite_functions(sqlite3 *db);
  void check_tables(sqlite3 *db);
  void sqlite_sha256(sqlite3_context *context, int argc, sqlite3_value **argv);
  void sqlite_encode_base32(sqlite3_context *context, int argc, sqlite3_value **argv);
  void sqlite_generate_salt(sqlite3_context *context, int argc, sqlite3_value **argv);
  int encode_base32(const unsigned char *src, size_t src_size, unsigned char *output, size_t output_size);
  int generate_salt(std::string &output, size_t len);
//  int sha256(const unsigned char *data, unsigned char *output);

#endif //SPHERE_LINKED_DATABASE_HPP
