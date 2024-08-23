#include "server_util.hpp"

void heartbeat(int &client_fd, sockaddr_in &client_addr,std::atomic<bool> &echo_heartbeat, std::atomic<bool> &close_session_flag) {
  char outbuffer[1024] = {0};
  Message heartbeat_message;
  heartbeat_message.type = HEARTBEAT;
  heartbeat_message.string = "";

  std::chrono::system_clock::time_point time_point;
  std::chrono::seconds duration;

  while (!close_session_flag) {

    try {
      send_message(client_fd, outbuffer, heartbeat_message);
      std::cout << "To " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " " \
 << "Sent: " << heartbeat_message.type << ", " << heartbeat_message.string << '\n';
    } catch (int err) {
      std::cerr << "Error sending heartbeat_message.\n";
    }

    // wait while no echo_heartbeat signal
    time_point = std::chrono::system_clock::now();
    echo_heartbeat = false;

    while (!echo_heartbeat) {
      duration = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now() - time_point);
      // if waiting duration > wait_time_sec, send close_session_flag
      if (duration > std::chrono::seconds(wait_time_sec)) { 
        close_session_flag = true;
        std::cout << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << ' ' \
          << "Heartbeat timeout.\n";
        break;
      }
    }

    // sleep
    std::this_thread::sleep_for(std::chrono::seconds(heartbeat_sleep_sec));
  }
}

void session(int client_fd, sockaddr_in client_addr) {
  std::atomic<bool> echo_heartbeat (false);
  std::atomic<bool> close_session_flag (false);

  fd_set read_fds;
  int ready_for_call;
  timeval timev;


  int nbytes = 0;
  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  std::thread heartbeat_thread(heartbeat, std::ref(client_fd), std::ref(client_addr), std::ref(echo_heartbeat), std::ref(close_session_flag));

  std::thread stream_thread;
  std::unordered_map<int, int> user_socket_fds(32);  // unordered_map[id] = socket_fd of user with {id}

  int new_port = 0;
  std::atomic<bool> port_connected(false);

  Message stream_port_message;
  stream_port_message.type = STREAM_PORT;

  while (!close_session_flag) {
    FD_ZERO(&read_fds);
    FD_SET(client_fd, &read_fds);
    timev.tv_sec = 0; timev.tv_usec = 0;


    ready_for_call = select(client_fd + 1, &read_fds, nullptr, nullptr, &timev);

    if (ready_for_call < 0) {
      std::cerr << "Error while using select().\n";
    } else if (ready_for_call == 0) {
      continue;
    } else {
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
      std::cout << "From " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " "     \
        << "Recv: " << message.type << ", " << message.string << '\n';

      if (message.type == CONNECT) {

        stream_thread = std::thread(stream_port, std::ref(client_fd), std::ref(new_port), std::ref(port_connected), std::ref(close_session_flag), std::ref(user_socket_fds));
        while (!port_connected) std::this_thread::yield;

        if (new_port != 0) {
          try {
            stream_port_message.string = std::to_string(new_port);
            send_message(client_fd, outbuffer, stream_port_message);

            std::cout << "To " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " " \
 << "Sent: " << stream_port_message.type << ", " << stream_port_message.string << '\n';
          } catch (int err) {
            std::cerr << "Error sending message.\n";
          }
        }

      }

      if (message.type == HEARTBEAT) echo_heartbeat = true;

      if (message.type == REDIRECT) {

      }

    }
  }

  heartbeat_thread.join();
  stream_thread.join();

  close(client_fd);
  std::cout << "Connection with " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " closed.\n";
}

void stream_port(int &client_fd, int &new_port, std::atomic<bool> &port_connected, std::atomic<bool> &close_session_flag, std::unordered_map<std::string, int> &user_socket_fds) {

  int stream_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  int stream_socket_status, on = 1;
  struct sockaddr_in server_addr_stream, client_addr_stream;

  server_addr_stream.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr_stream.sin_addr);


  for (int i = 0; i < available_port.size(); i++) {
    server_addr_stream.sin_port = htons(available_port[i]);

    stream_socket_status = bind(stream_socket_fd, (struct sockaddr *) &server_addr_stream, sizeof(server_addr_stream));
    if (stream_socket_status == -1) continue;

    int connection_limit = 1;
    stream_socket_status = listen(stream_socket_fd, connection_limit);

    if (stream_socket_status == -1) continue;
    if (setsockopt(stream_socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) continue;

    new_port = ntohs(server_addr_stream.sin_port);
    port_connected = true;
    std::cout << "Opened new port at: " << available_port[i] << '\n';
    available_port.erase(available_port.begin() + i);

    break;
  }

  if (new_port == 0) { std::cerr << "Unable to open new port.\n"; port_connected = true; return; }

  socklen_t client_stream_addrlen = sizeof(client_addr_stream);

  char stream_id_message_buffer[1024];
  Message stream_id_message;
  stream_id_message.type = REDIRECT;
  std::string new_key;

  while(!close_session_flag) {
    stream_socket_fd = accept(stream_socket_fd, (struct sockaddr*) &client_addr_stream, &client_stream_addrlen);

    new_key = random_key_gen();
    user_socket_fds.emplace(new_key, stream_socket_fd);
    stream_id_message.string = new_key;
    send_message(client_fd, stream_id_message_buffer, stream_id_message);

    std::cout << "Connection from " << inet_ntoa(client_addr_stream.sin_addr) << ':' << (int)ntohs(client_addr_stream.sin_port) << '\n';

  }
}

void proxy(int user_fd) {

}