//
// Created by Jhean Lee on 2025/2/15.
//

#include <iostream>
#include <iomanip>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>
#include <thread>

#include <fcntl.h>
#include <poll.h>
#include <termios.h>

#include "input.hpp"
#include "shared.hpp"
#include "console.hpp"
#include "../database/client.hpp"


static void print_help() {
  std::lock_guard<std::mutex> cout_lock(shared_resources::cout_mutex);
  std::cout << "\nCommands: \n"
               "Press \033[36mL\033[0m for a list of connected clients\n"
               "Press \033[36mU\033[0m for uptime status\n"
               "Press \033[36mH\033[0m for this help message\n\n";
}

static void print_uptime() {
  typedef std::chrono::duration<int, std::ratio<86400>> days;
  std::ostringstream buffer;
  buffer << '\n';

  char strtime[32];
  time_t start = std::chrono::system_clock::to_time_t(shared_resources::process_start);
  std::strftime(strtime, 32, "%Y-%m-%d %H:%M:%S", std::gmtime(&start));
  buffer << "Process started at: " << strtime << "(UTC)" << '\n';

  std::chrono::duration duration = std::chrono::system_clock::now() - shared_resources::process_start;
  days d = std::chrono::duration_cast<days>(duration);
  duration -= d;
  std::chrono::hours h = std::chrono::duration_cast<std::chrono::hours>(duration);
  duration -= h;
  std::chrono::minutes m = std::chrono::duration_cast<std::chrono::minutes>(duration);
  duration -= m;
  std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(duration);

  buffer << "Uptime: ";
  buffer << std::setfill('0') << d.count() << "d:" << std::setw(2) << h.count() << "h:" << std::setw(2) << m.count() << "m:" << std::setw(2) << s.count() << "s\n";

  buffer << '\n';
  std::lock_guard<std::mutex> cout_lock(shared_resources::cout_mutex);
  std::cout << buffer.str();
}

void input_thread_func() {
  //  configure stdin
  struct termios newt;
  newt = shared_resources::oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &newt);

  int flags = fcntl(0, F_GETFL, 0);
  fcntl(0, F_SETFL, flags | O_NONBLOCK);

  int in, status;
  struct pollfd pfd { .fd = 0, .events = POLLIN };

  print_help();
  while (!shared_resources::global_flag_kill) {
    status = poll(&pfd, 1, 1000);
    if (status <= 0) continue;

    in = std::cin.get();
    if (in == -1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      continue;
    }

    switch (in) {
      case 'l':
      case 'L':
        update_client_copy();
        list_clients();
        break;
      case 'u':
      case 'U':
        print_uptime();
        break;
      case 'h':
      case 'H':
        print_help();
        break;
      default:
        break;
    }
  }
}