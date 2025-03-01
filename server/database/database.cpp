//
// Created by Jhean Lee on 2025/1/21.
//
#include <iostream>
#include <random>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include "database.hpp"
#include "../common/shared.hpp"
#include "../tunnel/socket_management.hpp"
#include "../common/console.hpp"

void open_db(sqlite3 **db) {
  if (sqlite3_open(db_path, db) != SQLITE_OK) {
    console(ERROR, SQLITE_OPEN_FAILED, sqlite3_errmsg(*db), "database::open_db");
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }
}

void create_sqlite_functions(sqlite3 *db) {
  sqlite3_create_function(db, "sha256", 1, SQLITE_UTF8, nullptr, sqlite_sha256, nullptr, nullptr);
  sqlite3_create_function(db, "base32_encode", 1, SQLITE_UTF8, nullptr, sqlite_encode_base32, nullptr, nullptr);
  sqlite3_create_function(db, "generate_salt", 0, SQLITE_UTF8, nullptr, sqlite_generate_salt, nullptr, nullptr);
}

static int salt_callback(void *, int count, char **values, char **columns) {
  if (count != 1) return 1;
  shared_resources::db_salt = values[0];
  return 0;
}

void check_tables(sqlite3 *db) {
  char *errmsg = nullptr;

  //  authentication table
  const char *sql_auth = "CREATE TABLE IF NOT EXISTS auth("
                         "name TEXT PRIMARY KEY,"
                         "token TEXT,"
                         "notes TEXT,"
                         "expiry NUMERIC" //  seconds since epoch or null for no expiry
                         ");";
  if (sqlite3_exec(db, sql_auth, nullptr, nullptr, &errmsg) != SQLITE_OK) {
    console(ERROR, SQLITE_CREATE_TABLE_FAILED, errmsg, "database::check_tables");
    sqlite3_free(errmsg);
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }

  //  salt table
  const char *sql_salt = "CREATE TABLE IF NOT EXISTS salt("
                         "salt TEXT PRIMARY KEY"
                         ");";
  if (sqlite3_exec(db, sql_salt, nullptr, nullptr, &errmsg) != SQLITE_OK) {
    console(ERROR, SQLITE_CREATE_TABLE_FAILED, errmsg, "database::check_tables");
    sqlite3_free(errmsg);
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }
  //  generate salt if not exist
  const char *sql_salt_exist = "INSERT INTO salt (salt)"
                               "SELECT generate_salt()"
                               "WHERE NOT EXISTS(SELECT 1 FROM salt);";
  if (sqlite3_exec(db, sql_salt_exist, nullptr, nullptr, &errmsg) != SQLITE_OK) {
    console(ERROR, SQLITE_RETRIEVE_FAILED, errmsg, "database::check_tables");
    sqlite3_free(errmsg);
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }
  //  get salt from db
  const char *sql_get_salt = "SELECT salt.salt FROM salt;";
  if (sqlite3_exec(db, sql_get_salt, salt_callback, nullptr, &errmsg) != SQLITE_OK) {
    console(ERROR, SQLITE_RETRIEVE_FAILED, errmsg, "database::check_tables");
    sqlite3_free(errmsg);
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }

  //  client table
  const char *sql_client = "CREATE TABLE IF NOT EXISTS client("
                           "ip TEXT PRIMARY KEY, "
                           "sent INTEGER, "
                           "received INTEGER"
                           ");";
  if (sqlite3_exec(db, sql_client, nullptr, nullptr, &errmsg) != SQLITE_OK) {
    console(ERROR, SQLITE_CREATE_TABLE_FAILED, errmsg, "database::check_tables");
    sqlite3_free(errmsg);
    cleanup_openssl();
    exit(EXIT_FAILURE);
  }
}

void sqlite_sha256(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc != 1) {
    sqlite3_result_error(context, "SHA256 expects 1 argument\n", -1);
    return;
  }
  const unsigned char *data = sqlite3_value_text(argv[0]);

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (ctx == nullptr) {
    sqlite3_result_error(context, "failed to initialise SHA256 context\n", -1);
    return;
  }
  if (!EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr)) {
    sqlite3_result_error(context, "failed to set up SHA256 context\n", -1);
    EVP_MD_CTX_free(ctx);
    return;
  }
  if (!EVP_DigestUpdate(ctx, data, strlen(reinterpret_cast<const char *>(data)))) {
    sqlite3_result_error(context, "failed to update SHA256 context\n", -1);
    EVP_MD_CTX_free(ctx);
    return;
  }

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len;
  if (!EVP_DigestFinal_ex(ctx, hash, &hash_len)) {
    sqlite3_result_error(context, "failed to finalise SHA256 context\n", -1);
    EVP_MD_CTX_free(ctx);
    return;
  }

  sqlite3_result_blob(context, hash, hash_len, SQLITE_TRANSIENT);
  EVP_MD_CTX_free(ctx);
}

void sqlite_encode_base32(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc != 1) {
    sqlite3_result_error(context, "base32 encoding expects 1 argument\n", -1);
    return;
  }
  const unsigned char *data = static_cast<const unsigned char *>(sqlite3_value_blob(argv[0]));
  size_t data_len = sqlite3_value_bytes(argv[0]);

  unsigned char encoded[64];
  int encoded_len = encode_base32(data, data_len, encoded, sizeof(encoded));

  if (encoded_len < 0) {
    sqlite3_result_error(context, "base32 encoding failed\n", -1);
    return;
  }

  sqlite3_result_text(context, reinterpret_cast<const char *>(encoded), encoded_len, SQLITE_TRANSIENT);
}

void sqlite_generate_salt(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc != 0) {
    sqlite3_result_error(context, "salt generation expects no argument", -1);
    return;
  }

  std::string salt;
  if (generate_salt(salt, 8) < 0) {
    sqlite3_result_error(context, "salt generation failed", -1);
    return;
  }

  sqlite3_result_text(context, salt.c_str(), 8, SQLITE_TRANSIENT);
}

static const char base32_encoding_table[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
int encode_base32(const unsigned char *src, size_t src_size, unsigned char *output, size_t output_size) {
  if (output_size < (src_size * 8 + 4) / 5) return -1;

  int output_index = 0;
  int bit_buffer = 0;
  int bit_counter = 0;

  for (int i = 0; i < src_size; i++) {
    bit_buffer = (bit_buffer << 8) | src[i];
    bit_counter += 8;

    while (bit_counter >= 5) {
      if (output_index >= output_size) return -1;
      output[output_index++] = base32_encoding_table[(bit_buffer >> (bit_counter - 5)) & 0x1F];
      bit_counter -= 5;
    }
  }

  if (bit_counter > 0) {
    if (output_index >= output_size) return -1;
    output[output_index++] = base32_encoding_table[(bit_buffer << (5 - bit_counter)) & 0x1F];
  }

  while (output_index % 8 != 0) {
    if (output_index >= output_size) return -1;
    output[output_index++] = '=';
  }

  return output_index;
}

static const char symbols[65] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+/";
int generate_salt(std::string &output, size_t len) {
  unsigned char buf[len];
  output = "";
  if (RAND_bytes(buf, len) != 1) {
    console(ERROR, RAND_FAILED, nullptr, "database::generate_salt");
    return -1;
  }
  for (int i = 0; i < len; i++) {
    output.push_back(symbols[(int) buf[i] % 64]);
  }
  return 0;
}

//int sha256(const unsigned char *data, unsigned char *output) {
//  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
//  if (ctx == nullptr) {
//    console(ERROR, SHA256_INIT_CONTEXT_FAILED, nullptr, "database::sha256");
//    return 1;
//  }
//  if (!EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr)) {
//    console(ERROR, SHA256_SET_CONTEXT_FAILED, nullptr, "database::sha256");
//    EVP_MD_CTX_free(ctx);
//    return 1;
//  }
//  if (!EVP_DigestUpdate(ctx, data, strlen(reinterpret_cast<const char *>(data)))) {
//    console(ERROR, SHA256_UPDATE_CONTEXT_FAILED, nullptr, "database::sha256");
//    EVP_MD_CTX_free(ctx);
//    return 1;
//  }
//
//  unsigned char hash[EVP_MAX_MD_SIZE];
//  unsigned int hash_len;
//  if (!EVP_DigestFinal_ex(ctx, hash, &hash_len)) {
//    console(ERROR, SHA256_FINALISE_CONTEXT_FAILED, nullptr, "database::sha256");
//    EVP_MD_CTX_free(ctx);
//    return 1;
//  }
//
//  for (int i = 0; i < hash_len; i++) {
//    output[i] = hash[i];
//  }
//
//  EVP_MD_CTX_free(ctx);
//  return 0;
//}