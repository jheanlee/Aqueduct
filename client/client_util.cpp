#include "client_util.hpp"

extern int host_stream_port;

void heartbeat(int &socket_fd, char *outbuffer) {
//  char inbuffer[1024] = {0}, outbuffer[1024] = {0};
  Message message;
  message.type = HEARTBEAT;
  message.string = "";

  send_message(socket_fd, outbuffer, message);
  std::cout << "Sent: " << message.type << ", " << message.string << '\n';
}

void stream_service() {
  int server_socket_fd, server_status;
  int service_socket_fd, service_status;

  service_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (service_socket_fd == -1) { std::cerr << "Failed to create socket.\n"; exit(1); }

  struct sockaddr_in service_addr;
  service_addr.sin_family = AF_INET;
  inet_pton(AF_INET, client, &service_addr.sin_addr);
  service_addr.sin_port = htons(client_stream_port);

  char service_inbuffer[stream_io_buffer_size] = {0}, service_outbuffer[stream_io_buffer_size] = {0};
  service_status = connect(service_socket_fd, (struct sockaddr *) &service_addr, sizeof(service_addr));
  if (service_status == -1) { std::cerr << "Service connection error.\n"; exit(1); }

  //TODO
  std::cout << host_stream_port << '\n';

}

