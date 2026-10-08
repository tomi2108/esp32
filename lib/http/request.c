#include "request.h"
#include "response.h"
#include <stdio.h>
#include <string.h>

void request_destroy(Request *request) {
  buffer_destroy(request->buffer);
  // should probably free headers but client could reuse them as well...
  if (request->body)
    body_destroy(request->body);
  free(request);
}

void request_parse(Request *req) {
  const char *body_separator = "\r\n\r\n";
  char *stream = buffer_read_string(req->buffer);
  if (stream == NULL)
    return;

  char *body_start = strstr(stream, body_separator);
  body_start[0] = '\0';
  char *body = body_start + 4;
  body[strlen(body) - 1] = '\0';
  body_add_string(req->body, body, strlen(body));

  char *end_str;
  char *line = strtok_r(stream, "\r\n", &end_str);

  // TODO: Do something with version probably... ?
  char version[16] = {0};
  sscanf(line, "%7s %1023s %15s", req->method, req->path, version);

  line = strtok_r(NULL, "\r\n", &end_str);
  while (line != NULL) {
    char *end_token;
    char *header = malloc(strlen(line) + 1);
    strcpy(header, line);
    char *key = strtok_r(header, ":", &end_token);
    char *value = strtok_r(NULL, "", &end_token);
    if (value[0] == ' ')
      value = &value[1];

    headers_add(req->headers, key, value);
    line = strtok_r(NULL, "\r\n", &end_str);
    free(header);
  }
  free(stream);
}

Request *request_create() {
  Request *request = malloc(sizeof(Request));
  if (request == NULL)
    return NULL;

  request->buffer = buffer_create();
  request->headers = headers_create();
  request->body = body_create();
  return request;
}

Request *request_recieve(int socket) {
  Request *request = request_create();

  // TODO: Using fixed sizes seems off... read RFC: 2616
  // may need to implement a proper parser =)
  char stream[1024] = {0};
  int recieved = read(socket, stream, 1000);
  stream[recieved] = 0;
  buffer_add_string(request->buffer, recieved, stream);

  request_parse(request);
  return request;
}

Response *request_send(Request *req, int socket) {
  t_buffer *buf = buffer_create();
  buffer_add_string(buf, strlen(req->method), req->method);
  buffer_add_string(buf, 1, " ");
  buffer_add_string(buf, strlen(req->path), req->path);
  buffer_add_string(buf, 9, " HTTP/1.1");
  buffer_add_string(buf, 2, "\r\n");

  t_list_iterator *headers_iterator = list_iterator_create(req->headers);
  while (list_iterator_has_next(headers_iterator)) {
    Header *header = list_iterator_next(headers_iterator);
    buffer_add_string(buf, strlen(header->key), header->key);
    buffer_add_string(buf, 2, ": ");
    buffer_add_string(buf, strlen(header->value), header->value);
    buffer_add_string(buf, 2, "\r\n");
  }
  list_iterator_destroy(headers_iterator);

  if (req->body && strlen(req->body->stream) > 0) {
    buffer_add_string(buf, 2, "\r\n");
    buffer_add_string(buf, req->body->size, req->body->stream);
    buffer_add_string(buf, 2, "\r\n");
  }

  buffer_add_string(buf, 2, "\r\n");

  write(socket, buf->stream, buf->size);

  buffer_destroy(buf);
  request_destroy(req);
  Response *res = response_recieve(socket);
  return res;
}
