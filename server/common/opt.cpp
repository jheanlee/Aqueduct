//
// Created by Jhean Lee on 2024/12/10.
//
#include <string>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#include "opt.hpp"
#include "shared.hpp"
#include "console.hpp"
#include "../database/authentication.hpp"
#include "../database/database.hpp"
#include "signal_handler.hpp"

int ssl_control_port = 30330;
int proxy_port_start = 51000;
int proxy_port_limit = 200;
int timeout_session_millisec = 10;
int timeout_proxy_millisec = 1;
std::string db_path_str = "./sphere-linked.sqlite";
std::string key_path_str;
std::string cert_path_str;
const char *cert_path = "\0";
const char *key_path = "\0";
const char *db_path = "./sphere-linked.sqlite";
bool verbose = false;

void opt_handler(int argc, char * const argv[]) {
  std::string name, notes;

  CLI::App app{"Sphere-Linked-server"};
  app.get_formatter()->column_width(35);
  app.require_subcommand(1, 1);

  app.add_flag("-v,--verbose", verbose, "Output detailed information");
  app.add_option("-d,--database", db_path_str, "The path to database file")->capture_default_str();

  CLI::App *run = app.add_subcommand("run", "Run the main tunneling service")->fallthrough();
  run->add_option("-k,--tls-key", key_path_str, "The path to a private key file used for TLS encryption")->required();
  run->add_option("-c,--tls-cert", cert_path_str, "The path to a certification file used for TLS encryption")->required();

  run->add_option("-p,--control", ssl_control_port, "Client will connect via 0.0.0.0:<port>")->capture_default_str();
  run->add_option("-s,--port-start", proxy_port_start, "The proxy port of the first client will be <port>, the last being (<port> + port-limit - 1)")->capture_default_str();
  run->add_option("-l,--port-limit", proxy_port_limit, "Proxy ports will have a limit of <count> ports")->capture_default_str();

  run->add_option("--session-timeout", timeout_session_millisec, "The time(ms) poll() waits each call when accepting connections. See `man poll` for more information")->capture_default_str();
  run->add_option("--proxy-timeout", timeout_proxy_millisec, "The time(ms) poll() waits each call during proxying. See `man poll` for more information")->capture_default_str();

  CLI::App *token = app.add_subcommand("token", "Operations related to tokens")->require_subcommand(1, 1);

  CLI::App *token_new = token->add_subcommand("new", "Create or regenerate a token")->fallthrough();
  token_new->add_option("-n,--name", name, "The name (id) of the token you want to modify")->required();
  token_new->add_option("--notes", notes, "Some notes for this token");

  CLI::App *token_remove = token->add_subcommand("remove", "Remove a token")->fallthrough();
  token_remove->add_option("-n,--name", name, "The name (id) of the token you want to modify")->required();

  CLI::App *token_list = token->add_subcommand("list", "List all tokens")->fallthrough();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    exit(app.exit(e));
  }

  db_path = db_path_str.c_str();
  key_path = key_path_str.c_str();
  cert_path = cert_path_str.c_str();

  if (*token) {
    open_db(&shared_resources::db);
    create_sqlite_functions(shared_resources::db);
    check_tables(shared_resources::db);
    if (*token_new) {
      int status = new_token(name, notes);
      signal_handler(status);
    } else if (*token_remove) {
      int status = remove_token(name);
      signal_handler(status);
    } else if (*token_list) {
      int status = list_token();
      signal_handler(status);
    }
  }

  //  port validation
  if (proxy_port_start <= 0 || proxy_port_start > 65535) {
    console(ERROR, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }
  if (proxy_port_start < 1024) {
    console(WARNING, PORT_WELL_KNOWN, nullptr, "opt::opt_handler");
  }

  if (proxy_port_limit < 1) {
    console(ERROR, PORT_INVALID_LIMIT, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }
  if (proxy_port_start + proxy_port_limit - 1 > 65535) {
    console(WARNING, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
  }

  if (ssl_control_port <= 0 || ssl_control_port > 65535) {
    console(ERROR, PORT_INVALID_RANGE, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }
  if (ssl_control_port < 1024) {
    console(WARNING, PORT_WELL_KNOWN, nullptr, "opt::opt_handler");
  }

  //  TLS
  if (key_path_str.empty()) {
    console(ERROR, OPTION_KEY_NOT_SET, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }
  if (cert_path_str.empty()) {
    console(ERROR, OPTION_CERT_NOT_SET, nullptr, "opt::opt_handler");
    exit(EXIT_FAILURE);
  }

  console(INFO, INFO_CERT_PATH, cert_path, "opt::opt_handler");
  console(INFO, INFO_KEY_PATH, key_path, "opt::opt_handler");
  console(INFO, INFO_DB_PATH, db_path, "opt::opt_handler");
  console(INFO, INFO_HOST, (std::string(host) + ':' + std::to_string(ssl_control_port)).c_str(), "opt::opt_handler");
}
