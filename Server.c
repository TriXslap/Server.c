#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_NUM 8080
#define BACKLOG 10

int send_all(int socket_fd, const void *buffer, size_t length) {
  const char *buf = buffer;
  size_t total_sent = 0;

  while (total_sent < length) {
    ssize_t sent = send(socket_fd, buf + total_sent, length - total_sent, 0);
    if (sent < 0) {
      perror("send");
      return -1;
    }
    if (sent == 0) {
      return -1;
    }
    total_sent += (size_t)sent;
  }
  return 0;
}

int send_response(int socket_fd) {
  char *error_response = "HTTP/1.1 404 Not Found\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 14\r\n"
                         "Connection: close\r\n"
                         "\r\n"
                         " 404 Not found\r\n";

  FILE *html_file = fopen("./index.html", "r");
  if (html_file == NULL) {
    perror("fopen");
    return -1;
  }
  char file_buf[1024];
  memset(file_buf, 0, sizeof(file_buf));
  size_t bytesRead;
  char *http_header = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: 333\r\n"
                      "Connection: close\r\n"
                      "\r\n";

  send(socket_fd, http_header, strlen(http_header), 0);
  while ((bytesRead = fread(file_buf, 1, sizeof(file_buf), html_file)) > 0) {
    send_all(socket_fd, file_buf, bytesRead);
  }
  fclose(html_file);
  return 0;
}

int handle_request(int socket_fd) {
  char recived_buf[2048];
  memset(recived_buf, 0, sizeof(recived_buf));
  ssize_t bytesRead = recv(socket_fd, recived_buf, sizeof(recived_buf) - 1, 0);
  printf("---- NEW MASSEGE RECEIVED ----\n%s\n-----------------------\n",
         recived_buf);

  send_response(socket_fd);
  close(socket_fd);
  return 0;
}

int server_loop(int socket_fd) {
  while (1) {
    int sock = accept(socket_fd, NULL, NULL);
    if (sock == -1) {
      perror("accept");
      return -1;
    }
    handle_request(sock);
  }
  close(socket_fd);
  return 0;
}

int main(int argc, char **argv) {
  int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listening_socket <= -1) {
    perror("socket");
    return -1;
  }

  int enable = 1;
  setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable,
             sizeof(enable));

  struct sockaddr_in server_adress = {AF_INET, htons(PORT_NUM),
                                      htonl(INADDR_ANY)};
  if (bind(listening_socket, (struct sockaddr *)&server_adress,
           sizeof(server_adress)) == -1) {
    perror("bind");
    close(listening_socket);
    return -1;
  }

  int is_listening;
  if ((is_listening = listen(listening_socket, BACKLOG)) != 0) {
    perror("listen");
    close(listening_socket);
    return -1;
  }

  server_loop(listening_socket);
  return 0;
}
