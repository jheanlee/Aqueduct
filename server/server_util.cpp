#include "server_util.hpp"

void heartbeat(int &client_fd, sockaddr_in &client_addr,std::atomic<bool> &echo_heartbeat, std::atomic<bool> &close_session_flag) {
  char outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";

  std::chrono::system_clock::time_point time_point;
  std::chrono::seconds duration;

  while (!close_session_flag) {

    try {
      send_message(client_fd, outbuffer, message);
      std::cout << "To " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " " \
        << "Sent: " << message.type << ", " << message.string << '\n';
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
  int new_port = 0;
  std::atomic<bool> port_connected(false);
  Message redirect_message;
  redirect_message.type = REDIRECT;

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

        stream_thread = std::thread(stream_port, std::ref(new_port), std::ref(port_connected));
        while (!port_connected) std::this_thread::yield;

        if (new_port != 0) {
          try {
            redirect_message.string = std::to_string(new_port);
            send_message(client_fd, outbuffer, redirect_message);

            std::cout << "To " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " " \
        << "Sent: " << redirect_message.type << ", " << redirect_message.string << '\n';
          } catch (int err) {
            std::cerr << "Error sending message.\n";
          }
        }

      }

      if (message.type == HEARTBEAT) echo_heartbeat = true;

    }
  }

  heartbeat_thread.join();
  stream_thread.join();

  close(client_fd);
  std::cout << "Connection with " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " closed.\n";
}

void stream_port(int &new_port, std::atomic<bool> &port_connected) {
  int stream_fd = socket(AF_INET, SOCK_STREAM, 0);
  int status, on = 1;
  struct sockaddr_in server_addr_stream, client_addr_stream;

  server_addr_stream.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr_stream.sin_addr);


  for (int i = 0; i < available_port.size(); i++) {
    server_addr_stream.sin_port = htons(available_port[i]);

    status = bind(stream_fd, (struct sockaddr *) &server_addr_stream, sizeof(server_addr_stream));
    if (status == -1) continue;

    int connection_limit = 1;
    status = listen(stream_fd, connection_limit);

    if (status == -1) continue;
    if (setsockopt(stream_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) continue;

    new_port = ntohs(server_addr_stream.sin_port);
    port_connected = true;
    std::cout << "Opened new port at: " << available_port[i] << '\n';
    available_port.erase(available_port.begin() + i);

    break;
  }

  if (new_port == 0) { std::cerr << "Unable to open new port.\n"; port_connected = true; return; }

  socklen_t client_stream_addrlen = sizeof(client_addr_stream);

  stream_fd = accept(stream_fd, (struct sockaddr*) &client_addr_stream, &client_stream_addrlen);
  std::cout << "Connection from " << inet_ntoa(client_addr_stream.sin_addr) << ':' << (int)ntohs(client_addr_stream.sin_port) << '\n';

  // TODO: [note] tell client the port, client connect to port 3000. After connected, client send redirect request (probably with an id), server redirect to new port

}