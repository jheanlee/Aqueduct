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

  std::thread listen_port_thread;

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
        listen_port_thread = std::thread(listen_new_port);
      }

      if (message.type == HEARTBEAT) echo_heartbeat = true;

    }
  }

  heartbeat_thread.join();
  listen_port_thread.join();

  close(client_fd);
  std::cout << "Connection with " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << " closed.\n";
}

void listen_new_port() {
  int new_fd = socket(AF_INET, SOCK_STREAM, 0);
  int status, on = 1;
  struct sockaddr_in new_server_addr, new_client_addr;

  new_server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, host, &new_server_addr.sin_addr);


  for (int i = 0; i < available_port.size(); i++) {
    new_server_addr.sin_port = htons(available_port[i]);

    status = bind(new_fd, (struct sockaddr *) &new_server_addr, sizeof(new_server_addr));
    if (status == -1) { continue; }

    int connection_limit = 1;
    status = listen(new_fd, connection_limit);

    if (status == -1) { continue; }
    if (setsockopt(new_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) { continue; }
    
    std::cout << "Opened port at: " << available_port[i] << '\n';
    available_port.erase(available_port.begin() + i);
    break;
  }

  socklen_t new_client_addrlen = sizeof(new_client_addr);

  new_fd = accept(new_fd, (struct sockaddr*) &new_client_addr, &new_client_addrlen);
  std::cout << "Connection from " << inet_ntoa(new_client_addr.sin_addr) << ':' << (int)ntohs(new_client_addr.sin_port) << '\n';

  // TODO: [note] tell client the port, client connect to port 3000. After connected, client send redirect request (probably with an id), server redirect to new port
}