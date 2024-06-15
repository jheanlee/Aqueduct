#include "message.hpp"
#include "server_util.hpp"
#include "config.hpp"

int main() {
  int socket_fd = 0;  //socket file descriptor
  int status;
  int on = 1;

  init_available_port();

  // socket creation (AF_INET = ipv4, SOCK_STREAM = tcp)
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd == -1) { std::cerr << "Failed to create socket.\n"; exit(1); }


  // server address
  struct sockaddr_in server_addr, client_addr;

  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(main_port);


  // bind socket
  status = bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  if (status == -1) { std::cerr << "Binding Error.\n"; exit(1); }


  // listening
  // accept up to {connection_limit} connections
  int connection_limit = 1;
  status = listen(socket_fd, connection_limit);

  if (status == -1) { std::cerr << "Listening Error.\n"; exit(1); }
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) { std::cerr << "Setsockopt Error. \n"; exit(1); }

  std::cout << "Waiting for connection...\n";
  socklen_t client_addrlen;
  client_addrlen = sizeof(client_addr);

  int client_fd;
  while (true) {
    client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addrlen);
    std::cout << "Connection from " << inet_ntoa(client_addr.sin_addr) << ':' << (int)ntohs(client_addr.sin_port) << '\n';

    std::thread new_session_thread(session, client_fd, client_addr);
    new_session_thread.detach();
  }
 
  return 0;
}