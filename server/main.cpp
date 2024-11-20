//
// Created by Jhean Lee on 2024/10/2.
//

#include "message.hpp"
#include "config.hpp"
#include "connection.hpp"

int main() {
  int socket_fd = 0, status = 0, on = 1;

  struct sockaddr_in server_addr{.sin_family = AF_INET, .sin_port = htons(main_port)}, client_addr;
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  std::unordered_map<std::string, std::pair<int, sockaddr_in>> external_user_id_map; // id, {fd, addr}

  init_proxy_port_available();

  //  create socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tcp
  if (socket_fd == -1) { std::cerr << "[main.cpp] Failed to create socket.\n"; exit(1);}

  //  bind socket
  status = bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (status == -1) { std::cerr << "[main.cpp] Binding error.\n"; exit(1); }

  // listeniing
  status = listen(socket_fd, connection_limit);
  if (status == -1) { std::cerr << "[main.cpp] Listening error.\n"; exit(1); }
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) { std::cerr << "[main.cpp] Setsockopt error. \n"; exit(1); }

  std::cout << "Waiting for connection...\n";
  socklen_t client_addrlen = sizeof(client_addr);

  int client_fd;
  while (true) {
    client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addrlen);
    std::cout << "Connection from " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << '\n';

    std::thread session_thread(session_thread_func, client_fd, client_addr, ref(external_user_id_map));
    session_thread.detach();
  }

  return 0;
}