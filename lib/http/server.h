#ifndef SERVER_H_
#define SERVER_H_

#include "buffer.h"
#include "list.h"
#include "request.h"
#include "response.h"
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef void (*Handler)(Request *req, Response *res);

typedef struct Endpoint {
  Path path;
  Method method;
  Handler handler;
} Endpoint;

typedef struct HttpServer {
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
