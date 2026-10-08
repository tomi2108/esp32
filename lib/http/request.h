#ifndef REQUEST_H_
#define REQUEST_H_

#include "buffer.h"
#include "list.h"
#include "protocol.h"
#include "response.h"
#include <netdb.h>
#include <unistd.h>

typedef struct Request {
  t_buffer *buffer;
  char method[16];
  char path[1024];
  Body body;
  Headers headers;
} Request;

Request *request_create();
void request_destroy(Request *request);

Response *request_send(Request *req, int socket);
Request *request_recieve(int socket);

#endif
