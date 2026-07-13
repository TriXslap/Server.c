#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_NUM 8080
#define BACKLOG 10

int send_response(int socket_fd) {
  FILE *html_file = fopen("./index.html", "r");
  if (html_file == NULL) {
    perror("error ecourd while trying to open ./index.html");
    return -1;
  }
  char file_buf[1024];
  memset(file_buf, 0, sizeof(file_buf));
  size_t bytesRead;
  char *http_header = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "\r\n";

  send(socket_fd, http_header, strlen(http_header), 0);
  while ((bytesRead = fread(file_buf, 1, sizeof(file_buf), html_file)) > 0) {
    send(socket_fd, file_buf, bytesRead, 0);
  }
  fclose(html_file);
  return 0;
}

int handle_request(int socket_fd) {
  char recived_buf[2048];
  memset(recived_buf, 0, sizeof(recived_buf));
  size_t bytesRead = recv(socket_fd, recived_buf, sizeof(recived_buf) - 1, 0);
  printf("---- NEW MASSEGE RECIVED ----\n%s\n-----------------------\n",
         recived_buf);

  send_response(socket_fd);
  close(socket_fd);
  return 0;
}

int server_loop(int socket_fd) {
  while (1) {
    int sock = accept(socket_fd, NULL, NULL);
    if (sock == -1) {
      perror("error ecurred while the program tried the accept a socket");
      return -1;
    }
    handle_request(sock);
  }
  close(socket_fd);
  return 0;
}

int main(int argc, char **argv) {
  int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listeningSocket <= -1) {
    perror("failed to create the listening socket");
    return -1;
  }

  struct sockaddr_in serverAdress = {AF_INET, htons(PORT_NUM),
                                     htonl(INADDR_ANY)};
  if (bind(listeningSocket, (struct sockaddr *)&serverAdress,
           sizeof(serverAdress)) == -1) {
    perror("error acourd while binding the lsitening socket");
    return -1;
  }
  listen(listeningSocket, BACKLOG);

  server_loop(listeningSocket);
  return 0;
}
