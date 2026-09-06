#include "http.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int validate_request(http_response_t *http_response,
                     http_header_t http_header) {
  if (strcmp(http_header.method, "GET") != 0) {
    fprintf(stderr, "the method: {%s} is not allowd", http_header.method);
    return 405;
  }
  if (access(http_response->resolved_path, F_OK) != 0) {
    fprintf(stderr, "the path: {%s} doesnt exist", http_header.path);
    return 404;
  }
  if (strcmp(http_header.version, "HTTP/1.1")) {
    fprintf(stderr, "the version: {%s}, is not allowd on this website",
            http_header.version);
    return 400;
  }
  return 200;
}

int parse_request(http_header_t *http_header, char *received_buf) {
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

int handle_request(int socket_fd) {

  char received_buf[2048];
  http_header_t http_header;
  http_response_t http_response;
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

  if (strstr(received_buf, "\r\n\r\n") != NULL) {
    printf("---- NEW MESSAGE RECEIVED ----\n%s\n-----------------------\n",
           received_buf);
    if (parse_request(&http_header, received_buf) == -1) {
      fprintf(stderr, "error ecourd in function parse_request");
      return -1;
    }
    if (resolve_path(&http_response, sizeof(http_response.resolved_path),
                     http_header) == -1) {
      fprintf(stderr, "error ecourd in function resolve_request");
      return -1;
    }

    http_response.status_code = validate_request(&http_response, http_header);
    resolve_content_type(&http_response);

    if (send_response(&http_response, socket_fd) == -1) {
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
