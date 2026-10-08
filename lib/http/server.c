#include "server.h"
#include "response.h"

HttpServer *server_create() {
  HttpServer *server = malloc(sizeof(HttpServer));
  if (server == NULL)
    return NULL;

  server->endpoints = list_create();
  return server;
}

void server_endpoint(HttpServer *server, Method method, Path path,
                     Handler handler) {
  Endpoint *endpoint = malloc(sizeof(Endpoint));
  if (endpoint == NULL)
    return;

  endpoint->method = method;
  endpoint->path = path;
  endpoint->handler = handler;

  list_add(server->endpoints, endpoint);
}

int server_listen(HttpServer *server, char *port, void (*callback)()) {
  // consider moving to server_init && HttpServer struct
  struct addrinfo hints;
  struct addrinfo *server_info;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int err_get_addr = getaddrinfo(NULL, port, &hints, &server_info);
  if (err_get_addr != 0)
    return -1;

  int fd_socket = socket(server_info->ai_family, server_info->ai_socktype,
                         server_info->ai_protocol);
  if (fd_socket == -1)
    return -1;

  int err_bind = bind(fd_socket, server_info->ai_addr, server_info->ai_addrlen);
  if (err_bind == -1)
    return -1;

  int err_listen = listen(fd_socket, 5);
  if (err_listen == -1)
    return -1;

  freeaddrinfo(server_info);

  callback();
  while (1) {
    int client_socket = accept(fd_socket, NULL, NULL);
    if (client_socket == -1)
      continue;

    Request *req = request_recieve(client_socket);
    Response *res = response_create();

    t_list_iterator *iterator = list_iterator_create(server->endpoints);
    while (list_iterator_has_next(iterator)) {
      Endpoint *endpoint = list_iterator_next(iterator);
      if (strcmp(endpoint->path, req->path) == 0 &&
          strcmp(endpoint->method, req->method) == 0) {
        endpoint->handler(req, res);
        response_send(res, client_socket);
        break;
      }
    }
    close(client_socket);
  }
  close(fd_socket);
}

void server_get(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "GET", path, handler);
}

void server_post(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "POST", path, handler);
}

void server_put(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "PUT", path, handler);
}

void server_patch(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "PATCH", path, handler);
}

void server_delete(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "DELETE", path, handler);
}

void server_options(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "OPTIONS", path, handler);
}

void server_head(HttpServer *server, Path path, Handler handler) {
  return server_endpoint(server, "HEAD", path, handler);
}
