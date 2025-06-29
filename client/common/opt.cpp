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
#include "signal_handler.hpp"
#include "shared.hpp"

std::string readable_host_str = "0.0.0.0";
const char *host = "0.0.0.0";
const char *readable_host = "0.0.0.0";
int host_main_port = 30330;

std::string local_service_str = "0.0.0.0";
const char *local_service = "0.0.0.0";
int local_service_port = -1;

int timeout_proxy_millisec = 1;
char hostname[NI_MAXHOST];
std::regex reg_ipv4(R"((\d{1,3}(\.\d{1,3}){3}))");
std::regex reg_token("AQ_[A-Za-z0-9+/]{32}");
std::string token;
int verbose_level = 20;

void opt_handler(int argc, char * const argv[]) {
  struct addrinfo *result;
  struct addrinfo *addr_ptr;
  struct addrinfo hint;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_INET;
  int error;

  CLI::App app{"Aqueduct-client"};
  app.get_formatter()->column_width(35);

  app.add_option("-v,--verbose", verbose_level, "Output information detail level (inclusive). 10 for Debug or above, 50 for Critical only. Daemon logs have mask of max(30, verbose_level)")->capture_default_str();
  app.add_option("-t,--token", token, "Token for accessing server. Only use this option on trusted machine");
  app.add_flag("-D, --daemon-mode", config::daemon_mode, "Disables stdout and use syslog or os_log instead")->capture_default_str();

  app.add_option("-H,--host-addr", readable_host_str, "The host to stream to. Accepts ipv4 or domain")->capture_default_str();
  app.add_option("-P,--host-port", host_main_port, "The control port of host")->capture_default_str();
  app.add_option("-s,--service-addr", local_service_str, "The address of the service to be tunnelled")->capture_default_str();
  app.add_option("-p,--service-port", local_service_port, "The port of the service to be tunnelled")->required();

  app.add_option("--proxy-timeout", timeout_proxy_millisec, "the time (ms) poll() waits when no data is available (smaller value means more frequent switch between user and service)")->capture_default_str();
  
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    signal_handler(app.exit(e));
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
      signal_handler(EXIT_FAILURE);
    }
    addr_ptr = result;
    while (addr_ptr != nullptr) {
      error = getnameinfo(addr_ptr->ai_addr, addr_ptr->ai_addrlen, hostname, NI_MAXHOST, nullptr, 0, 0);
      if (error != 0) {
        console(ERROR, RESOLVE_HOST_FAILED, nullptr, "option");
        signal_handler(EXIT_FAILURE);
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
    signal_handler(EXIT_FAILURE);
  }
  if (local_service_port <= 0 || local_service_port > 65535) {
    console(ERROR, PORT_INVALID_RANGE, nullptr, "option");
    signal_handler(EXIT_FAILURE);
  }

  if (token.empty()) {
    console(INSTRUCTION, ENTER_TOKEN_INSTRUCTION, nullptr, "option");
    std::cin >> token;
  }

  token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
  token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
  if (!std::regex_match(token, reg_token)) {
    console(ERROR, INVALID_TOKEN, nullptr, "option");
    signal_handler(EXIT_FAILURE);
  }

  console(NOTICE, INFO_HOST, (std::string(readable_host) + '(' + std::string(host) + ')' + ':' + std::to_string(host_main_port)).c_str(), "option");
  console(NOTICE, INFO_SERVICE, (std::string(local_service) + ':' + std::to_string(local_service_port)).c_str(), "option");
}