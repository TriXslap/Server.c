#include "http.h"

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_NUM 8080
#define BACKLOG 10

int server_loop(int socket_fd) {
  while (1) {
    int sock = accept(socket_fd, NULL, NULL);
    if (sock == -1) {
      perror("accept");
      return -1;
    }
    if (handle_request(sock) == -1) {
      fprintf(stderr, "failed to handle client");
    }
    close(sock);
  }
  close(socket_fd);
  return 0;
}

int main(int argc, char **argv) {
  int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
    perror("socket");
    return -1;
  }

  int enable = 1;
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable,
                 sizeof(enable)) == -1) {
    perror("setsockopt");
    close(listening_socket);
    return 0;
  }

  struct sockaddr_in server_address = {.sin_family = AF_INET,
                                       .sin_port = htons(PORT_NUM),
                                       .sin_addr.s_addr = htonl(INADDR_ANY)};
  if (bind(listening_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) == -1) {
    perror("bind");
    close(listening_socket);
    return -2;
  }

  if ((listen(listening_socket, BACKLOG)) == -1) {
    perror("listen");
    close(listening_socket);
    return -2;
  }

  server_loop(listening_socket);
  return -1;
}
