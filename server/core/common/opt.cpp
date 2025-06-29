//
// Created by Jhean Lee on 2024/12/10.
//
#include <string>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <sys/stat.h>

#include "opt.hpp"
#include "shared.hpp"
#include "console.hpp"
#include "../database/authentication.hpp"
#include "../database/database.hpp"
#include "signal_handler.hpp"
#include "../key/generation.hpp"
#include "../database/web_user.hpp"

int ssl_control_port = 30330;
int proxy_port_start = 51000;
int proxy_port_limit = 200;
int timeout_proxy_millisec = 1;
std::string db_path_str = "./aqueduct.sqlite";
const char *db_path = "./aqueduct.sqlite";
int verbose_level = 20;
bool shared_resources::daemon_mode = false;

void opt_handler(int argc, char * const argv[]) {
  int expiry_days = 100;
  std::string name, notes;

  CLI::App app{"Aqueduct-server"};
  app.get_formatter()->column_width(35);
  app.require_subcommand(1, 1);

  app.add_option("-v,--verbose", verbose_level, "Output information detail level (inclusive). 10 for Debug or above, 50 for Critical only. Daemon logs have mask of max(30, verbose_level)")->capture_default_str();
  app.add_option("-d,--database", db_path_str, "The path to database file")->capture_default_str();

  //  run
  CLI::App *run = app.add_subcommand("run", "Run the tunnelling service")->fallthrough();
  run->add_flag("-D, --daemon-mode", shared_resources::daemon_mode, "Disables stdout and use syslog or os_log instead")->capture_default_str();

  CLI::Option *ssl_private_key = run->add_option("--ssl-private-key", config::ssl_private_key_path_str, "The path to a private key file used for TLS encryption");
  CLI::Option *ssl_cert = run->add_option("--ssl-cert", config::ssl_cert_path_str, "The path to a certification file used for TLS encryption");
  CLI::Option *jwt_access_private_key = run->add_option("--jwt-access-private-key", config::jwt_access_private_key_path_str, "The path to a private key file used for JWT encoding (access token)");
  CLI::Option *jwt_access_public_key = run->add_option("--jwt-access-public-key", config::jwt_access_public_key_path_str, "The path to a public key file used for JWT decoding (access token)");
  CLI::Option *jwt_refresh_private_key = run->add_option("--jwt-refresh-private-key", config::jwt_refresh_private_key_path_str, "The path to a private key file used for JWT encoding (refresh token)");
  CLI::Option *jwt_refresh_public_key = run->add_option("--jwt-refresh-public-key", config::jwt_refresh_public_key_path_str, "The path to a public key file used for JWT decoding (refresh token)");
  ssl_private_key->needs(ssl_cert);
  ssl_cert->needs(ssl_private_key);
  jwt_access_private_key->needs(jwt_access_public_key);
  jwt_access_public_key->needs(jwt_access_private_key);
  jwt_refresh_private_key->needs(jwt_refresh_public_key);
  jwt_refresh_public_key->needs(jwt_refresh_private_key);

  run->add_option("-p,--control", ssl_control_port, "Client will connect via 0.0.0.0:<port>")->capture_default_str();
  run->add_option("-s,--port-start", proxy_port_start, "The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)")->capture_default_str();
  run->add_option("-l,--port-limit", proxy_port_limit, "Proxy ports will have a limit of <count> ports")->capture_default_str();

  run->add_option("--proxy-timeout", timeout_proxy_millisec, "The time (ms) poll() waits when no data is available (smaller value means more frequent switch between user and service)")->capture_default_str();

  //  token
  CLI::App *token = app.add_subcommand("token", "Token management")->require_subcommand(1, 1)->fallthrough();

  CLI::App *token_new = token->add_subcommand("new", "Create or regenerate a token")->fallthrough();
  token_new->add_option("-n,--name", name, "The name (id) of the token you want to modify")->required();
  token_new->add_option("--notes", notes, "Some notes for this token");
  token_new->add_option("--expiry", expiry_days, "Days until the expiry of the token. 0 for no expiry")->capture_default_str()->check(CLI::Range(0, 3650));

  CLI::App *token_remove = token->add_subcommand("remove", "Remove a token")->fallthrough();
  token_remove->add_option("-n,--name", name, "The name (id) of the token you want to modify")->required();

  CLI::App *token_list = token->add_subcommand("list", "List all tokens")->fallthrough();

  //  web-user
  CLI::App *web_user = app.add_subcommand("web-user", "User management for webui")->require_subcommand(1, 1)->fallthrough();

  CLI::App *web_user_new = web_user->add_subcommand("new", "Create a new user")->fallthrough();
  CLI::App *web_user_modify = web_user->add_subcommand("modify", "Modify a user")->fallthrough();
  CLI::App *web_user_remove = web_user->add_subcommand("remove", "Remove a user")->fallthrough();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    signal_handler(app.exit(e));
  }

  db_path = db_path_str.c_str();

  //  token actions
  if (*token) {
    open_db(&shared_resources::db);
    create_sqlite_functions(shared_resources::db);
    check_tables(shared_resources::db);
    if (*token_new) {
      int status = new_token(name, notes, expiry_days);
      signal_handler(status);
    } else if (*token_remove) {
      int status = remove_token(name);
      signal_handler(status);
    } else if (*token_list) {
      int status = list_token();
      signal_handler(status);
    }
  }

  //  web-user actions
  if (*web_user) {
    open_db(&shared_resources::db);
    create_sqlite_functions(shared_resources::db);
    check_tables(shared_resources::db);
    if (*web_user_new) {
      signal_handler(new_user());
    } else if (*web_user_modify) {
      signal_handler(modify_user());
    } else if (*web_user_remove) {
      signal_handler(remove_user());
    }
  }

  //  key/cert generation
  struct stat st;
  if (config::ssl_private_key_path_str.empty()) {
    config::ssl_private_key_path_str = "./credentials/aqueduct-ssl-private.pem";
    config::ssl_cert_path_str = "./credentials/aqueduct-ssl-cert";

    if (stat("./credentials/aqueduct-ssl-private.pem", &st) != 0 || stat("./credentials/aqueduct-ssl-cert", &st) != 0) {
      if (mkdir("./credentials", 0700) != 0 && errno != EEXIST) {
        console(CRITICAL, CREATE_DIR_FAILED, std::to_string(errno).c_str(), "opt::opt_handler");
        signal_handler(EXIT_FAILURE);
      }
      if (generate_ssl_key_cert("./credentials") != 0) {
        signal_handler(EXIT_FAILURE);
      }
    }
  }

  if (config::jwt_access_private_key_path_str.empty()) {
    config::jwt_access_private_key_path_str = "./credentials/aqueduct-jwt-access-private.pem";
    config::jwt_access_public_key_path_str = "./credentials/aqueduct-jwt-access-public.pem";

    if (stat("./credentials/aqueduct-jwt-access-private.pem", &st) != 0 || stat("./credentials/aqueduct-jwt-access-public.pem", &st) != 0) {
      if (mkdir("./credentials", 0700) != 0 && errno != EEXIST) {
        console(CRITICAL, CREATE_DIR_FAILED, std::to_string(errno).c_str(), "opt::opt_handler");
        signal_handler(EXIT_FAILURE);
      }
      if (generate_jwt_key_pair("./credentials", "aqueduct-jwt-access") != 0) {
        signal_handler(EXIT_FAILURE);
      }
    }
  }

  if (config::jwt_refresh_private_key_path_str.empty()) {
    config::jwt_refresh_private_key_path_str = "./credentials/aqueduct-jwt-refresh-private.pem";
    config::jwt_refresh_public_key_path_str = "./credentials/aqueduct-jwt-refresh-public.pem";

    if (stat("./credentials/aqueduct-jwt-refresh-private.pem", &st) != 0 || stat("./credentials/aqueduct-jwt-refresh-public.pem", &st) != 0) {
      if (mkdir("./credentials", 0700) != 0 && errno != EEXIST) {
        console(CRITICAL, CREATE_DIR_FAILED, std::to_string(errno).c_str(), "opt::opt_handler");
        signal_handler(EXIT_FAILURE);
      }
      if (generate_jwt_key_pair("./credentials", "aqueduct-jwt-refresh") != 0) {
        signal_handler(EXIT_FAILURE);
      }
    }
  }

  config::ssl_cert_path = config::ssl_cert_path_str.c_str();
  config::ssl_private_key_path = config::ssl_private_key_path_str.c_str();
  config::jwt_access_public_key_path = config::jwt_access_public_key_path_str.c_str();
  config::jwt_access_private_key_path = config::jwt_access_private_key_path_str.c_str();
  config::jwt_refresh_public_key_path = config::jwt_refresh_public_key_path_str.c_str();
  config::jwt_refresh_private_key_path = config::jwt_refresh_private_key_path_str.c_str();

  //  port validation
  if (proxy_port_start <= 0 || proxy_port_start > 65535) {
    console(CRITICAL, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
    signal_handler(EXIT_FAILURE);
  }
  if (proxy_port_start < 1024) {
    console(WARNING, PORT_WELL_KNOWN, nullptr, "opt::opt_handler");
  }

  if (proxy_port_limit < 1) {
    console(CRITICAL, PORT_INVALID_LIMIT, nullptr, "opt::opt_handler");
    signal_handler(EXIT_FAILURE);
  }
  if (proxy_port_start + proxy_port_limit - 1 > 65535) {
    console(WARNING, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
  }

  if (ssl_control_port <= 0 || ssl_control_port > 65535) {
    console(CRITICAL, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
    signal_handler(EXIT_FAILURE);
  }
  if (ssl_control_port < 1024) {
    console(WARNING, PORT_WELL_KNOWN, nullptr, "opt::opt_handler");
  }

  console(NOTICE, INFO_HOST, (std::string(host) + ':' + std::to_string(ssl_control_port)).c_str(), "opt::opt_handler");
  console(NOTICE, INFO_DB_PATH, db_path, "opt::opt_handler");
  console(NOTICE, INFO_JWT_ACCESS_PUBKEY_PATH, config::jwt_access_public_key_path, "opt::opt_handler");
  console(NOTICE, INFO_JWT_ACCESS_PRIVKEY_PATH, config::jwt_access_private_key_path, "opt::opt_handler");
  console(NOTICE, INFO_JWT_REFRESH_PUBKEY_PATH, config::jwt_refresh_public_key_path, "opt::opt_handler");
  console(NOTICE, INFO_JWT_REFRESH_PRIVKEY_PATH, config::jwt_refresh_private_key_path, "opt::opt_handler");
  console(NOTICE, INFO_SSL_KEY_PATH, config::ssl_private_key_path, "opt::opt_handler");
  console(NOTICE, INFO_SSL_CERT_PATH, config::ssl_cert_path, "opt::opt_handler");
}
