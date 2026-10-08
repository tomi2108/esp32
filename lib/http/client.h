#ifndef CLIENT_H_
#define CLIENT_H_

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

typedef struct HttpClient {
  Headers headers;
} HttpClient;

HttpClient *client_create();
void client_destroy(HttpClient *client);

Response *client_send_get(HttpClient *client, char *path);
Response *client_send_post(HttpClient *client, Body body, char *path);
Response *client_send_put(HttpClient *client, char *path);
Response *client_send_patch(HttpClient *client, char *path);
Response *client_send_delete(HttpClient *client, char *path);
Response *client_send_options(HttpClient *client, char *path);
Response *client_send_head(HttpClient *client, char *path);

#endif
