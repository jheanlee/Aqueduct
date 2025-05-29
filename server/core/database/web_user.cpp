//
// Created by Jhean Lee on 2025/5/20.
//
#include <iostream>
#include <string>
#include <regex>

#include <sqlite3.h>

#include "web_user.hpp"
#include "../common/console.hpp"
#include "../common/shared.hpp"
#include "database.hpp"

static const std::regex regex_username(R"(^[a-zA-Z][a-zA-Z0-9]{0,31}$)");
static const std::regex regex_password(R"(^[!-~]{1,32}$)");

int new_user() {
  std::string username, password;

  //  username
  console(INSTRUCTION, USERNAME_NEW_INSTRUCTION, nullptr, "web_user::new_user");
  std::cin >> username;
  if (!std::regex_match(username, regex_username)) {
    console(ERROR, USERNAME_INVALID, nullptr, "web_user::new_user");
    return 1;
  }

  //  check if username exists
  const char *check_sql = "SELECT EXISTS("
                    "SELECT web_auth.username FROM web_auth "
                    "WHERE web_auth.username = ?"
                    ") AS sql_result;";
  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::new_user");
    return 1;
  }

  if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)  {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::new_user");
    sqlite3_finalize(stmt);
    return 1;
  }

  int sql_result = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    sql_result = sqlite3_column_int(stmt, 0);
  } else {
    console(ERROR, SQLITE_RETRIEVE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::new_user");
    return 1;
  }
  sqlite3_finalize(stmt);

  if (sql_result == 1) {
    console(ERROR, USERNAME_USED, nullptr, "web_user::new_user");
    return 1;
  }

  //  password
  console(INSTRUCTION, PASSWORD_NEW_INSTRUCTION, nullptr, "web_user::new_user");
  std::cin >> password;
  if (!std::regex_match(password, regex_password)) {
    console(ERROR, PASSWORD_INVALID, nullptr, "web_user::new_user");
    return 1;
  }

  //  salt
  std::string salt;
  if (generate_salt(salt, 8) < 0) {
    console(ERROR, RAND_FAILED, nullptr, "web_user::new_user");
    return 1;
  }

  //  insert into db
  const char *insert_sql = "INSERT INTO web_auth (username, hashed_password, salt) VALUES (?, base32_encode(sha256(? || ?)), ?);";
  stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(CRITICAL, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::new_user");
    return -1;
  }

  if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 2, salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 4, salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK
  ) {
    console(CRITICAL, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::new_user");
    sqlite3_finalize(stmt);
    return 1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(CRITICAL, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::new_user");
  }

  sqlite3_finalize(stmt);
  console(NOTICE, USER_CREATED, username.c_str(), "web_user::new_user");
  return 0;
}

int modify_user() {
  std::string username, password;

  //  username
  console(INSTRUCTION, USERNAME_MODIFY_INSTRUCTION, nullptr, "web_user::modify_user");
  std::cin >> username;
  if (!std::regex_match(username, regex_username)) {
    console(ERROR, USERNAME_INVALID, nullptr, "web_user::modify_user");
    return 1;
  }

  //  check if username exists
  const char *check_sql = "SELECT EXISTS("
                          "SELECT web_auth.username FROM web_auth "
                          "WHERE web_auth.username = ?"
                          ") AS sql_result;";
  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::modify_user");
    return 1;
  }

  if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)  {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::modify_user");
    sqlite3_finalize(stmt);
    return 1;
  }

  int sql_result = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    sql_result = sqlite3_column_int(stmt, 0);
  } else {
    console(ERROR, SQLITE_RETRIEVE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::modify_user");
    return 1;
  }
  sqlite3_finalize(stmt);

  if (sql_result != 1) {
    console(ERROR, USERNAME_NOT_FOUND, nullptr, "web_user::modify_user");
    return 1;
  }

  //  password
  console(INSTRUCTION, PASSWORD_NEW_INSTRUCTION, nullptr, "web_user::modify_user");
  std::cin >> password;
  if (!std::regex_match(password, regex_password)) {
    console(ERROR, PASSWORD_INVALID, nullptr, "web_user::modify_user");
    return 1;
  }

  //  salt
  std::string salt;
  if (generate_salt(salt, 8) < 0) {
    console(ERROR, RAND_FAILED, nullptr, "web_user::modify_user");
    return 1;
  }

  //  update db
  const char *update_sql = "UPDATE web_auth "
                           "SET hashed_password = base32_encode(sha256(? || ?)), "
                           "salt = ? "
                           "WHERE username = ?;";
  stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, update_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(CRITICAL, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::modify_user");
    return -1;
  }

  if (sqlite3_bind_text(stmt, 1, salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 4, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK
  ) {
    console(CRITICAL, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::modify_user");
    sqlite3_finalize(stmt);
    return 1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(CRITICAL, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::modify_user");
  }

  sqlite3_finalize(stmt);
  console(NOTICE, USER_MODIFIED, username.c_str(), "web_user::modify_user");
  return 0;
}

int remove_user() {
  std::string username, double_check;

  //  username
  console(INSTRUCTION, USERNAME_REMOVE_INSTRUCTION, nullptr, "web_user::remove_user");
  std::cin >> username;
  if (!std::regex_match(username, regex_username)) {
    console(ERROR, USERNAME_INVALID, nullptr, "web_user::remove_user");
    return 1;
  }

  //  check if username exists
  const char *check_sql = "SELECT EXISTS("
                          "SELECT web_auth.username FROM web_auth "
                          "WHERE web_auth.username = ?"
                          ") AS sql_result;";
  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(ERROR, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::remove_user");
    return 1;
  }

  if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)  {
    console(ERROR, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::remove_user");
    sqlite3_finalize(stmt);
    return 1;
  }

  int sql_result = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    sql_result = sqlite3_column_int(stmt, 0);
  } else {
    console(ERROR, SQLITE_RETRIEVE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::remove_user");
    return 1;
  }
  sqlite3_finalize(stmt);

  if (sql_result != 1) {
    console(ERROR, USERNAME_NOT_FOUND, nullptr, "web_user::remove_user");
    return 1;
  }

  //  double check
  console(INSTRUCTION, REMOVE_DOUBLE_CHECK_INSTRUCTION, nullptr, "web_user::remove_user");
  std::cin >> double_check;
  std::transform(double_check.cbegin(), double_check.cend(), double_check.begin(), [](unsigned char c){ return tolower(c); });
  if (double_check == "n" || double_check == "no") {
    return 1;
  } else if (double_check != "y" && double_check != "yes") {
    console(ERROR, UNKNOWN_OPTION, nullptr, "web_user::remove_user");
    return 1;
  }

  const char *delete_sql = "DELETE FROM web_auth WHERE web_auth.username = ?;";

  stmt = nullptr;

  if (sqlite3_prepare_v2(shared_resources::db, delete_sql, -1, &stmt, nullptr) != SQLITE_OK) {
    console(CRITICAL, SQLITE_PREPARE_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::remove_user");
    return 1;
  }

  if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    console(CRITICAL, SQLITE_BIND_PARAMETER_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::remove_user");
    sqlite3_finalize(stmt);
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    console(CRITICAL, SQLITE_STEP_FAILED, sqlite3_errmsg(shared_resources::db), "web_user::remove_user");
  }

  sqlite3_finalize(stmt);
  console(NOTICE, USER_REMOVED, nullptr, "web_user::remove_user");
  return 0;
}