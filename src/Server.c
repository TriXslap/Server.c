#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_NUM 8080
#define BACKLOG 10

typedef struct HttpHeader {
  char method[16];
  char path[256];
  char version[16];
} http_header_t;

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

int send_response(int socket_fd, char *file_path) {
  char *error_response = "HTTP/1.1 404 Not Found\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 15\r\n"
                         "Connection: close\r\n"
                         "\r\n"
                         "404 Not found\r\n";

  FILE *html_file = fopen(file_path, "rb");
  if (html_file == NULL) {
    printf("file_path: %s\n", file_path);
    send_all(socket_fd, error_response, strlen(error_response));
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

  if (header_length < 0 || (size_t)header_length >= sizeof(http_header)) {
    fprintf(stderr, "failed to build the HTTP header\n");
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
  if (ferror(html_file)) {
    perror("fread");
    fclose(html_file);
    return -1;
  }
  fclose(html_file);
  return 0;
}

int parse_request(http_header_t *http_header, char *received_buf) {
  printf("we got to the parse request");
  char *header = strtok(received_buf, "\r\n");
  char *tok_pointer = strtok(header, " ");
  if (tok_pointer == NULL || strlen(tok_pointer) >= 16) {
    fprintf(stderr, "failed to find the header method");
    return -1;
  }
  strcpy(http_header->method, tok_pointer);
  tok_pointer = strtok(NULL, " ");
  if (tok_pointer == NULL || strlen(tok_pointer) >= 256) {
    fprintf(stderr, "failed to find the path path");
    return -1;
  }
  strcpy(http_header->path, tok_pointer);
  tok_pointer = strtok(NULL, " ");
  if (tok_pointer == NULL || strlen(tok_pointer) >= 16) {
    fprintf(stderr, "failed to find the version method");
    return -1;
  }
  strcpy(http_header->version, tok_pointer);

  return 0;
}

int resolve_request(char *file_path, int file_path_size,
                    http_header_t http_header) {
  int bytes_read;
  if (strcmp(http_header.path, "/") == 0) {
    bytes_read = snprintf(file_path, file_path_size, "public/index.html");
  } else {
    bytes_read =
        snprintf(file_path, file_path_size, "public%s", http_header.path);
  }
  if (bytes_read < 0) {
    fprintf(stderr, "error while resolving the request");
    return -1;
  } else if (bytes_read >= file_path_size) {
    fprintf(stderr, "the path variable is to small\n");
    return bytes_read;
  }
  return 0;
}
int handle_request(int socket_fd) {
  char received_buf[2048];
  http_header_t http_header;
  char file_path[256];
  memset(received_buf, 0, sizeof(received_buf));
  ssize_t bytes_read = 0;
  while (strstr(received_buf, "\r\n\r\n") == NULL &&
         bytes_read < sizeof(received_buf) - 1) {
    ssize_t cur_bytes = recv(socket_fd, received_buf + bytes_read,
                             sizeof(received_buf) - bytes_read - 1, 0);
    if (cur_bytes < 0) {
      perror("recv");
      close(socket_fd);
      return -1;
    }
    if (cur_bytes == 0) {
      close(socket_fd);
      return 0;
    }
    bytes_read = bytes_read + cur_bytes;
    received_buf[bytes_read] = '\0';
  }

  printf("ended loop\n bytes_read: %d, sizeof(received_buf): %d\n", bytes_read,
         sizeof(received_buf));
  if (strstr(received_buf, "\r\n\r\n") != NULL) {
    printf("were good\n");
    printf("---- NEW MESSAGE RECEIVED ----\n%s\n-----------------------\n",
           received_buf);
    printf("before the parsing function\n");
    if (parse_request(&http_header, received_buf) == -1) {
      fprintf(stderr, "error ecourd in function parse_request");
      return -1;
    }
    if (resolve_request(file_path, 256, http_header) == -1) {
      fprintf(stderr, "error ecourd in function resolve_request");
    }

    if (send_response(socket_fd, file_path) == -1) {
      close(socket_fd);
      return -1;
    }

    close(socket_fd);
    return 0;
  }
  fprintf(stderr, "message is too big for the recived_buf");
  close(socket_fd);
  return -1;
}

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

  if ((listen(listening_socket, BACKLOG)) != 0) {
    perror("listen");
    close(listening_socket);
    return -1;
  }

  server_loop(listening_socket);
  return 0;
}
