//
// Created by Jhean Lee on 2024/12/2.
//
#include <cstdlib>
#include <regex>
#include <iostream>

#include <netdb.h>
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#include "opt.hpp"
#include "console.hpp"

std::string readable_host_str = "0.0.0.0";
const char *host = "0.0.0.0";
const char *readable_host = "0.0.0.0";
int host_main_port = 30330;

std::string local_service_str = "0.0.0.0";
const char *local_service = "0.0.0.0";
int local_service_port = -1;

int timeout_session_millisec = 10;
int timeout_proxy_millisec = 1;
char hostname[NI_MAXHOST];
std::regex reg_ipv4(R"((\d{1,3}(\.\d{1,3}){3}))");
std::regex reg_token("SL_[A-Za-z0-9+/]{32}");
std::string token;
bool verbose = false;

void opt_handler(int argc, char * const argv[]) {
  struct addrinfo *result;
  struct addrinfo *addr_ptr;
  struct addrinfo hint;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_INET;
  int error;

  CLI::App app{"Sphere-Linked-client"};
  app.get_formatter()->column_width(35);

  app.add_flag("-v,--verbose", verbose, "Output detailed information");
  app.add_option("-t,--token", token, "Token for accessing server. Only use this option on trusted machine");

  app.add_option("-H,--host-addr", readable_host_str, "The host to stream to. Accepts ipv4 or domain")->capture_default_str();
  app.add_option("-P,--host-port", host_main_port, "The control port of host")->capture_default_str();
  app.add_option("-s,--service-addr", local_service_str, "The address of the service to be tunneled")->capture_default_str();
  app.add_option("-p,--service-port", local_service_port, "The port of the service to be tunneled")->required();

  app.add_option("--session-timeout", timeout_session_millisec, "The time(ms) poll() waits each call when accepting connections. See `man poll` for more information")->capture_default_str();
  app.add_option("--proxy-timeout", timeout_proxy_millisec, "The time(ms) poll() waits each call during proxying. See `man poll` for more information")->capture_default_str();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    exit(app.exit(e));
  }

  readable_host = readable_host_str.c_str();
  local_service = local_service_str.c_str();

  //  handle ipv4 and domain
  if (std::regex_match(readable_host, reg_ipv4)) {
    host = readable_host;
  } else {
    error = getaddrinfo(readable_host, nullptr, &hint, &result);
    if (error != 0) {
      console(ERROR, RESOLVE_HOST_FAILED, nullptr, "option");
      exit(EXIT_FAILURE);
    }
    addr_ptr = result;
    while (addr_ptr != nullptr) {
      error = getnameinfo(addr_ptr->ai_addr, addr_ptr->ai_addrlen, hostname, NI_MAXHOST, nullptr, 0, 0);
      if (error != 0) {
        console(ERROR, RESOLVE_HOST_FAILED, nullptr, "option");
        exit(EXIT_FAILURE);
      }
      if (*hostname != '\0') {
        host = hostname;
        break;
      }
      addr_ptr = addr_ptr->ai_next;
    }
  }

  //  port validation
  if (host_main_port <= 0 || host_main_port > 65535) {
    console(ERROR, PORT_INVALID_RANGE, nullptr, "option");
    exit(EXIT_FAILURE);
  }
  if (local_service_port <= 0 || local_service_port > 65535) {
    console(ERROR, PORT_INVALID_RANGE, nullptr, "option");
    exit(EXIT_FAILURE);
  }

  if (token.empty()) {
    console(INSTRUCTION, ENTER_TOKEN_INSTRUCTION, nullptr, "option");
    std::cin >> token;
    if (!std::regex_match(token, reg_token)) {
      console(ERROR, INVALID_TOKEN, nullptr, "option");
      exit(EXIT_FAILURE);
    }
  }

  console(INFO, INFO_HOST, (std::string(readable_host) + '(' + std::string(host) + ')' + ':' + std::to_string(host_main_port)).c_str(), "option");
  console(INFO, INFO_SERVICE, (std::string(local_service) + ':' + std::to_string(local_service_port)).c_str(), "option");
}