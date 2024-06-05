#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include <unistd.h>

#include "message.hpp"

const int heartbeat_sleep_sec = 60;
const int wait_time_sec = 60;

void heartbeat(int &client_fd, std::atomic<bool> &echo_heartbeat, std::atomic<bool> &close_session_flag) {
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";

  std::chrono::system_clock::time_point time_point;
  std::chrono::seconds duration;

  while (!close_session_flag) {
    try {
      send_message(client_fd, outbuffer, message);
    } catch (int err) {
      std::cerr << "Error sending message.\n";
    }

    // wait while no echo_heartbeat signal
    time_point = std::chrono::system_clock::now();
    echo_heartbeat = false;

    while (!echo_heartbeat) {
      duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);
      // if waiting duration > wait_time_sec, send close_session_flag
      if (duration > std::chrono::seconds(wait_time_sec)) { 
        close_session_flag = true;
        std::cout << "Heartbeat timeout.\n";
        break;
      }
    }

    // sleep
    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
  }
}

void session(int &client_fd) {
  std::atomic<bool> echo_heartbeat (false);
  std::atomic<bool> close_session_flag (false);


  int nbytes;
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;

  std::thread heartbeat_thread(heartbeat, std::ref(client_fd), std::ref(echo_heartbeat), std::ref(close_session_flag));

  while (!close_session_flag) {   //TODO: resolve recv() blocking (possibly solve with select())
    try {
      // number of bytes received
      nbytes = recv_message(client_fd, inbuffer, message); 
    } catch (int err) {
      std::cerr << "Error receiving message.\n";
    }

    // nbytes == 0: client closed connection; nbytes == -1: error
    if (nbytes <= 0) {
      close_session_flag = true;
      break;
    }
    std::cout << "Received: " << message.type << ", " << message.string << '\n';

    // TODO: perform operation


  }

  heartbeat_thread.join();

  close(client_fd);
  std::cout << "Connection closed.\n";
}

