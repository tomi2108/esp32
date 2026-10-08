#include "response.h"
#include "list.h"
#include "protocol.h"

void response_parse(Response *res) {
  const char *body_separator = "\r\n\r\n";

  char *stream = buffer_read_string(res->buffer);
  if (stream == NULL)
    return;

  char *body_start = strstr(stream, body_separator);
  body_start[0] = '\0';
  char *body = body_start + 4;
  body[strlen(body) - 1] = '\0';
  body_add_string(res->body, body, strlen(body));

  char *end_str;
  char *line = strtok_r(stream, "\r\n", &end_str);

  // TODO: Do something with version probably... ?
  char version[16] = {0};
  char word[16] = {0};

  char status[16] = {0};
  sscanf(line, "%10s %10s %15s", version, status, word);
  res->status_code = atoi(status);

  line = strtok_r(NULL, "\r\n", &end_str);
  while (line != NULL) {
    char *end_token;
    char *header = malloc(strlen(line) + 1);
    strcpy(header, line);
    char *key = strtok_r(header, ":", &end_token);
    char *value = strtok_r(NULL, "", &end_token);
    if (value[0] == ' ')
      value = &value[1];

    headers_add(res->headers, key, value);
    line = strtok_r(NULL, "\r\n", &end_str);
    free(header);
  }
  free(stream);
};

void response_send(Response *res, int socket) {
  char *word = status[res->status_code];
  char status_code[6] = {0};
  snprintf(status_code, sizeof(status_code), "%d", res->status_code);

  t_buffer *buf = buffer_create();
  buffer_add_string(buf, 9, "HTTP/1.1 ");
  buffer_add_string(buf, strlen(status_code), status_code);
  buffer_add_string(buf, 1, " ");
  buffer_add_string(buf, strlen(word), word);
  buffer_add_string(buf, 2, "\r\n");

  t_list_iterator *headers_iterator = list_iterator_create(res->headers);
  while (list_iterator_has_next(headers_iterator)) {
    Header *header = list_iterator_next(headers_iterator);
    buffer_add_string(buf, strlen(header->key), header->key);
    buffer_add_string(buf, 2, ": ");
    buffer_add_string(buf, strlen(header->value), header->value);
    buffer_add_string(buf, 2, "\r\n");
  }
  list_iterator_destroy(headers_iterator);

  if (res->body->size > 0) {
    buffer_add_string(buf, 2, "\r\n");
    buffer_add_string(buf, res->body->size, res->body->stream);
    buffer_add_string(buf, 2, "\r\n");
  }

  write(socket, buf->stream, buf->size);

  buffer_destroy(buf);
  response_destroy(res);
}

Response *response_recieve(int socket) {
  Response *response = response_create();

  // TODO: Using fixed sizes seems off... read RFC: 2616
  // may need to implement a proper parser =)
  char stream[1024] = {0};
  int recieved = recv(socket, stream, 1000, MSG_WAITALL);
  stream[recieved] = 0;
  buffer_add_string(response->buffer, recieved, stream);

  response_parse(response);
  return response;
};

Response *response_create() {
  Response *response = malloc(sizeof(Response));
  if (response == NULL)
    return NULL;

  response->buffer = buffer_create();
  response->headers = headers_create();
  response->body = body_create();
  return response;
}

void response_destroy(Response *res) {
  if (res->body != NULL)
    body_destroy(res->body);
  if (res->headers != NULL)
    headers_destroy(res->headers);
  free(res);
}
