#ifndef SERVER_H_
#define SERVER_H_

#include "buffer.h"
#include "list.h"
#include "request.h"
#include "response.h"
#include "esp_http_server.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*Handler)(Request *req, Response *res);

typedef struct Endpoint {
  Path path;
  Method method;
  Handler handler;
  httpd_uri_t uri_cfg;
} Endpoint;

typedef struct HttpServer {
  httpd_handle_t handle;
  t_list *endpoints;
} HttpServer;

HttpServer *server_create();
int server_listen(HttpServer *server, char *port, void (*callback)());

void server_get(HttpServer *server, Path path, Handler handler);
void server_post(HttpServer *server, Path path, Handler handler);
void server_put(HttpServer *server, Path path, Handler handler);
void server_patch(HttpServer *server, Path path, Handler handler);
void server_delete(HttpServer *server, Path path, Handler handler);
void server_options(HttpServer *server, Path path, Handler handler);
void server_head(HttpServer *server, Path path, Handler handler);

#endif
