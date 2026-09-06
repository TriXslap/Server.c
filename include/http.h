#pragma once

#include <stddef.h>

typedef struct HttpHeader {
  char method[16];
  char path[256];
  char version[16];
} http_header_t;

typedef struct HttpResponse {
  char resolved_path[256];
  char content_type[64];
  int status_code;

} http_response_t;

typedef struct {
  const char *extension;
  const char *content_type;
} mime_type_t;

typedef struct {
  const int status_code;
  const char *message;
} error_message_t;

int handle_request(int socket_fd);

int resolve_path(http_response_t *http_response, int file_path_size,
                 http_header_t http_header);

int resolve_content_type(http_response_t *http_response);

int send_all(int socket_fd, const void *buffer, size_t length);

int send_response(http_response_t *http_response, int socket_fd);
