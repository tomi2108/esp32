#ifndef REQUEST_H_
#define REQUEST_H_

#include "buffer.h"
#include "list.h"
#include "protocol.h"
#include "response.h"
#include "esp_http_client.h"

typedef struct Request {
  char method[16];
  char path[1024];
  Body body;
  Headers headers;
} Request;

Request *request_create();
void request_destroy(Request *request);
Response *request_send(Request *req, const char *url);

#endif
