#include <stdio.h>
#include <iostream>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


using namespace std;


const char *host = "0.0.0.0";
const int port = 3000;

int main() {
  int socket_fd = 0;  //socket file descriptor
  int status;
  int on = 1;

  // socket creation (AF_INET = ipv4, SOCK_STREAM = tcp)
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd == -1) { cerr << "Failed to create socket.\n"; exit(1); }


  // server address
  struct sockaddr_in server_addr, client_addr;

  server_addr.sin_family = AF_INET;
  // inet_aton(host, &server_addr.sin_addr);
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);


  // bind socket
  status = bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  if (status == -1) { cerr << "Binding Error.\n"; exit(1); }


  // listening
  // accept up to {connection_limit} connections
  int connection_limit = 1;
  status = listen(socket_fd, connection_limit);

  if (status == -1) { cerr << "Listening Error.\n"; exit(1); }
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) { cerr << "Setsockopt Error. \n"; exit(1); }

  cout << "Waiting for connection...\n";
  socklen_t client_addrlen;
  client_addrlen = sizeof(client_addr);

  int client_fd;
  while (true) {
    client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addrlen);
    cout << "Connection from " << inet_ntoa(client_addr.sin_addr) << ':' << (int) ntohs(client_addr.sin_port) << '\n';


    char inbuffer[1024] = {0}, outbuffer[1024] = {0};
    while (true) {
      // receiving buffer
      int nbytes = recv(client_fd, inbuffer, sizeof(inbuffer), 0); // number of bytes received

      // nbytes == 0: client closed connection; nbytes == -1: error
      if (nbytes <= 0) { 
        close(client_fd);
        cout << "Connection closed.\n";
        break;
      }

      cout << "Received: " << inbuffer << '\n';

      // sending buffer (echo)
      sprintf(outbuffer, "echo %s\n", inbuffer); // writes echo {inbuffer} into outbuffer
      send(client_fd, outbuffer, strlen(outbuffer), 0);

    }
  }

  
  close(socket_fd);
  return 0;
}