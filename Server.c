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
                         "Content-Length: 15\r\n"
                         "Connection: close\r\n"
                         "\r\n"
                         "404 Not found\r\n";

  FILE *html_file = fopen("./index.html", "rb");
  if (html_file == NULL) {
    send_all(socket_fd, error_response, sizeof(error_response));
    perror("fopen");
    return -1;
  }
  if (fseek(html_file, 0, SEEK_END) == -1) {
    perror("fseek");
    fclose(html_file);
    return -1;
  }
  long file_size = ftell(html_file);
  if (file_size == -1) {
    perror("ftell");
    fclose(html_file);
    return -1;
  }
  rewind(html_file);
  char http_header[512];
  int header_length = snprintf(http_header, sizeof(http_header),
                               "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %ld\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               file_size);

  if (header_length < 0 || (size_t)header_length >= sizeof(http_header)){
    perror("snprintf");
    fclose(html_file);
    return -1;
  }
  if (send_all(socket_fd, http_header, header_length) == -1) {
    fclose(html_file);
    return -1;
  }

  size_t bytes_read;
  char file_buf[1024];
  memset(file_buf, 0, sizeof(file_buf));
  while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), html_file)) > 0) {
    if (send_all(socket_fd, file_buf, bytes_read) == -1) {
      fclose(html_file);
      return -1;
    }
  }
  if(ferror(html_file)) {
    perror("fread");
    fclose(html_file);
    return -1;
  }
  fclose(html_file);
  return 0;
}

int handle_request(int socket_fd) {
  char received_buf[2048];
  memset(received_buf, 0, sizeof(received_buf));
  ssize_t bytes_read =

      recv(socket_fd, received_buf, sizeof(received_buf) - 1, 0);
  if (bytes_read < 0) {
    perror("recv");
    close(socket_fd);
    return -1;
  }
  if (bytes_read == 0){
    close(socket_fd);
    return 0;
  }
  received_buf[bytes_read] = '\0';
  printf("---- NEW MESSAGE RECEIVED ----\n%s\n-----------------------\n",
         received_buf);

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
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable,
                 sizeof(enable)) == -1) {
    perror("setsockopt");
    close(listening_socket);
    return 1;
  }

  struct sockaddr_in server_address = {.sin_family = AF_INET,
                                       .sin_port = htons(PORT_NUM),
                                       .sin_addr.s_addr = htonl(INADDR_ANY)};
  if (bind(listening_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) == -1) {
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
