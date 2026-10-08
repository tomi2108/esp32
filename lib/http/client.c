#include "client.h"
#include "list.h"
#include "protocol.h"
#include "request.h"
#include "response.h"
#include <string.h>

HttpClient *client_create() {
  HttpClient *client = malloc(sizeof(HttpClient));
  client->headers = headers_create();
  return client;
}

void client_destroy(HttpClient *client) {
  if (client->headers) {
    headers_destroy(client->headers);
  }
}

Response *client_send(HttpClient *client, Method method, char *url, Body body) {
  char host[100] = {0};
  int p;
  char path[100] = {0};
  char port[100] = {0};

  sscanf(url, "http://%99[^:]:%d/%99[^\n]", host, &p, path);
  snprintf(port, 100, "%d", p);
  if (strlen(path) == 0)
    path[0] = '/';

  Request *req = request_create();
  req->headers = client->headers;
  req->body = body;
  strcpy(req->method, method);
  strcpy(req->path, path);

  // Consider  moving this to HttpClient client_create
  struct addrinfo hints;
  struct addrinfo *server_info;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int err_get_addr = getaddrinfo(host, port, &hints, &server_info);
  if (err_get_addr != 0)
    return NULL;

  int fd_socket = socket(server_info->ai_family, server_info->ai_socktype,
                         server_info->ai_protocol);

  if (fd_socket == -1)
    return NULL;

  int err_connect =
      connect(fd_socket, server_info->ai_addr, server_info->ai_addrlen);
  if (err_connect == -1)
    return NULL;

  freeaddrinfo(server_info);
  Response *res = request_send(req, fd_socket);
  return res;
}

Response *client_send_get(HttpClient *client, char *url) {
  return client_send(client, "GET", url, NULL);
}

Response *client_send_post(HttpClient *client, Body body, char *url) {
  return client_send(client, "POST", url, body);
}

Response *client_send_put(HttpClient *client, char *url) {
  return client_send(client, "PUT", url, NULL);
}

Response *client_send_patch(HttpClient *client, char *url) {
  return client_send(client, "PATCH", url, NULL);
}

Response *client_send_delete(HttpClient *client, char *url) {
  return client_send(client, "DELETE", url, NULL);
}

Response *client_send_options(HttpClient *client, char *url) {
  return client_send(client, "OPTIONS", url, NULL);
}

Response *client_send_head(HttpClient *client, char *url) {
  return client_send(client, "HEAD", url, NULL);
}
