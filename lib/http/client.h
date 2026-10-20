#ifndef CLIENT_H_
#define CLIENT_H_

#include "buffer.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "list.h"
#include "protocol.h"
#include "request.h"
#include "response.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct HttpClient {
  Headers headers;
  const char *baseUrl;
  SemaphoreHandle_t mutex;
} HttpClient;

void client_destroy(HttpClient *client);

HttpClient *client_create(const char *baseUrl);
Response *client_send_get(HttpClient *client, const char *url);
Response *client_send_post(HttpClient *client, Body body, const char *url);
Response *client_send_put(HttpClient *client, const char *url);
Response *client_send_patch(HttpClient *client, const char *url);
Response *client_send_delete(HttpClient *client, const char *url);
Response *client_send_options(HttpClient *client, const char *url);
Response *client_send_head(HttpClient *client, const char *url);

#endif
