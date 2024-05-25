#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


using namespace std;


const char *host = "0.0.0.0";
const int port = 3000;

int main() {
  int socket_fd = 0;  //socket file descriptor
  int status;

  // socket creation (AF_INET = ipv4, SOCK_STREAM = tcp)
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd == -1) { cerr << "Failed to create socket.\n"; exit(1); }


  // server address
  struct sockaddr_in server_addr, client_addr;

  server_addr.sin_family = AF_INET;
  inet_aton(host, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // bind socket
  status = bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  if (status == -1) { cerr << "Binding Error.\n"; exit(1); }

  // listening
  // accept up to {connection_limit} connections
  int connection_limit = 1;
  status = listen(socket_fd, connection_limit);

  if (status == -1) { cerr << "Listening Error.\n"; exit(1); }
  cout << "Waiting for connection..." << "\n";

  socklen_t client_addrlen;
  client_addrlen = sizeof(client_addr);

  int client_fd;
  while (true) {
    // TODO: finish
  }


  
  close(socket_fd);
  return 0;
}