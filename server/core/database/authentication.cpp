//
// Created by Jhean Lee on 2025/2/6.
//

#include <sqlite3.h>
#include <iostream>
#include <mutex>
#include <chrono>

#include "authentication.hpp"
#include "../common/shared.hpp"
#include "../common/console.hpp"
#include "database.hpp"

int new_token(const std::string &name, const std::string &notes, int expiry_days) {
  std::string buffer;
  if (generate_salt(buffer, 32) < 0) return -1;
  std::string token = "SL_" + buffer;
  std::chrono::system_clock::time_point time = std::chrono::system_clock::now() + std::chrono::hours (expiry_days * 24);
  std::chrono::duration duration = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch());

  std::string sql = "INSERT INTO auth (name, token, notes, expiry) VALUES (?, base32_encode(sha256(? || ?)), ?, ?) "
                    "ON CONFLICT (name) DO UPDATE "
                    "SET token = base32_encode(sha256(? || ?)), expiry = ?";
  sql += (notes.empty()) ? ";" : ", notes = ?;";

  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::new_token");
    return -1;
  }

  if (sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK   ||
      sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK  ||
      sqlite3_bind_text(stmt, 3, shared_resources::db_salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK  ||
      sqlite3_bind_text(stmt, 4, notes.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK  ||
      ((expiry_days != 0 && sqlite3_bind_int64(stmt, 5, duration.count()) != SQLITE_OK) || (expiry_days == 0 && sqlite3_bind_null(stmt, 5) != SQLITE_OK)) ||
      sqlite3_bind_text(stmt, 6, token.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK  ||
      sqlite3_bind_text(stmt, 7, shared_resources::db_salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK  ||
      ((expiry_days != 0 && sqlite3_bind_int64(stmt, 8, duration.count()) != SQLITE_OK) || (expiry_days == 0 && sqlite3_bind_null(stmt, 8) != SQLITE_OK)) ||
      (!notes.empty() && sqlite3_bind_text(stmt, 9, notes.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
  ) {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::new_token");
    sqlite3_finalize(stmt);
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(ERROR, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::new_token");
  }

  sqlite3_finalize(stmt);
  console(NOTICE, GENERATED_TOKEN, (name + ": " + token).c_str(), "authentication::new_token");
  return 0;
}

int remove_token(const std::string &name) {
  std::string sql = "DELETE FROM auth WHERE auth.name = ?;";

  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::remove_token");
    return -1;
  }

  if (sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::remove_token");
    sqlite3_finalize(stmt);
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(ERROR, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::remove_token");
  }

  sqlite3_finalize(stmt);
  console(NOTICE, REMOVED_TOKEN, nullptr, "authentication::remove_token");
  return 0;
}

static int list_callback(void *, int argc, char **argv, char **col_names) {
  std::lock_guard<std::mutex> cout_lock(shared_resources::cout_mutex);
  for (int i = 0; i < argc; i++) {
    std::cout << col_names[i] << ": " << ((argv[i]) ? argv[i] : "null") << '\n';
  }
  std::cout << '\n';
  return 0;
}

int list_token() {
  const char *sql = "SELECT * FROM auth;";
  char *errmsg;

  if (sqlite3_exec(shared_resources::db, sql, list_callback, nullptr, &errmsg) != SQLITE_OK) {
    console(ERROR, SQLITE_RETRIEVE_FAILED, errmsg, "authentication::list_token");
    sqlite3_free(errmsg);
  }

  return 0;
}

int check_token_expiry() {
  const char *delete_sql = "DELETE FROM auth WHERE expiry IS NOT NULL AND expiry <= ?;";
  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, delete_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::check_token_expiry");
    return -1;
  }

  if (sqlite3_bind_int64(stmt, 1, std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) != SQLITE_OK) {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::ckeck_token_expiry");
    sqlite3_finalize(stmt);
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(ERROR, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "authentication::ckeck_token_expiry");
  }

  sqlite3_finalize(stmt);
  return 0;
}