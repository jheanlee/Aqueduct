//
// Created by Jhean Lee on 2025/2/15.
//

#include <iostream>
#include <mutex>
#include <chrono>

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
               "Press \033[36mH\033[0m for this help message\n\n";
}

void input_thread_func() {
  //  configure stdin
  struct termios oldt, newt;
  tcgetattr(0, &oldt);
  newt = oldt;
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
    status = poll(&pfd, 1, 500);
    if (status <= 0) continue;

    in = std::cin.get();

    switch (in) {
      case 'l':
      case 'L':
        list_clients();
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