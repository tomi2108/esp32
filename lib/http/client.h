#ifndef CLIENT_H_
#define CLIENT_H_

#include "buffer.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "list.h"
#include "protocol.h"
#include "request.h"
#include "response.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct HttpClient {
  Headers headers;
  char *baseUrl;
} HttpClient;

HttpClient *client_create(char *baseUrl);
void client_destroy(HttpClient *client);

Response *client_send_get(HttpClient *client, char *path);
Response *client_send_post(HttpClient *client, Body body, char *path);
Response *client_send_put(HttpClient *client, char *path);
Response *client_send_patch(HttpClient *client, char *path);
Response *client_send_delete(HttpClient *client, char *path);
Response *client_send_options(HttpClient *client, char *path);
Response *client_send_head(HttpClient *client, char *path);

#endif
