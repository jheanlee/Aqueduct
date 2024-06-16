#ifndef TUNNEL_CONFIG_HPP
  #define TUNNEL_CONFIG_HPP

  #include <vector>
  #include <numeric>

static const char *host = "0.0.0.0";
static const int main_port = 3000;

static const int available_port_begin = 51000;
extern std::vector<int> available_port;
void init_available_port();

//server_util
static const int heartbeat_sleep_sec = 30;
static const int wait_time_sec = 30;

#endif
