#include "http.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
static const mime_type_t mime_types[] = {{".html", "text/html"},
                                         {".css", "text/css"},
                                         {".js", "text/javascript"},
                                         {".png", "image/png"},
                                         {".jpeg", "image/jpeg"}};

static const error_message_t error_messages[] = {
    {404, "Not Found"}, {400, "Bad Request"}, {405, "Method Not Allowed"}};

int resolve_path(http_response_t *http_response, int file_path_size,
                 http_header_t http_header) {
  int bytes_read;
  if (strcmp(http_header.path, "/") == 0) {
    bytes_read = snprintf(http_response->resolved_path, file_path_size,
                          "public/index.html");
  } else {
    bytes_read = snprintf(http_response->resolved_path, file_path_size,
                          "public%s", http_header.path);
  }
  if (bytes_read < 0) {
    fprintf(stderr, "error while resolving the request");
    return -1;
  } else if (bytes_read >= file_path_size) {
    fprintf(stderr, "the path variable is too small\n");
    return bytes_read;
  }
  return 0;
}

int resolve_content_type(http_response_t *http_response) {
  char *extension = strrchr(http_response->resolved_path, '.');
  if (extension == NULL) {
    fprintf(stderr, "no file extension in path");
    return -1;
  }
  for (int i = 0; i < (int)(sizeof(mime_types) / sizeof(mime_type_t)); i++) {
    if (strcmp(extension, mime_types[i].extension) == 0) {
      strcpy(http_response->content_type, mime_types[i].content_type);
      return 0;
    }
  }
  strcpy(http_response->content_type, "application/octet-stream");
  return -2;
}

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

int send_response(http_response_t *http_response, int socket_fd) {
  if (http_response->status_code != 200) {
    char body[64];
    char message[512];
    for (int i = 0; i < (int)(sizeof(error_messages) / sizeof(error_message_t));
         i++) {
      if (http_response->status_code == error_messages[i].status_code) {
        int body_len =
            snprintf(body, sizeof(body), "%d %s", error_messages[i].status_code,
                     error_messages[i].message);
        int message_len =
            snprintf(message, sizeof(message),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: %d\r\n"
                     "Connection: close\r\n"
                     "\r\n"
                     "%d %s\r\n",
                     error_messages[i].status_code, error_messages[i].message,
                     body_len + 2, error_messages[i].status_code,
                     error_messages[i].message);
        if (send_all(socket_fd, message, message_len) == -1) {
          return -1;
        }
      }
    }
    return 0;
  }

  FILE *html_file = fopen(http_response->resolved_path, "rb");
  if (html_file == NULL) {
    perror("fopen");
    return 1;
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
                               "HTTP/1.1 %d OK\r\n"
                               "Content-Type: %s\r\n"
                               "Content-Length: %ld\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               http_response->status_code,
                               http_response->content_type, file_size);

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
