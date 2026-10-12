#include "request.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  Response *response;
} http_event_ctx_t;

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  http_event_ctx_t *ctx = (http_event_ctx_t *)evt->user_data;
  switch (evt->event_id) {
  case HTTP_EVENT_ON_DATA:
    if (ctx && ctx->response && ctx->response->body) {
      buffer_add(ctx->response->body, evt->data, evt->data_len);
    }
    break;
  default:
    break;
  }
  return ESP_OK;
}

void request_destroy(Request *request) {
  if (request == NULL)
    return;
  if (request->body)
    body_destroy(request->body);
  free(request);
}

Request *request_create() {
  Request *request = malloc(sizeof(Request));
  if (request == NULL)
    return NULL;

  request->headers = headers_create();
  request->body = body_create();
  return request;
}

Response *request_send(Request *req, const char *url) {
  if (req == NULL || url == NULL) {
    request_destroy(req);
    return NULL;
  }

  esp_http_client_method_t method = HTTP_METHOD_GET;
  if (strcmp(req->method, "POST") == 0)
    method = HTTP_METHOD_POST;
  else if (strcmp(req->method, "PUT") == 0)
    method = HTTP_METHOD_PUT;
  else if (strcmp(req->method, "PATCH") == 0)
    method = HTTP_METHOD_PATCH;
  else if (strcmp(req->method, "DELETE") == 0)
    method = HTTP_METHOD_DELETE;
  else if (strcmp(req->method, "OPTIONS") == 0)
    method = HTTP_METHOD_OPTIONS;
  else if (strcmp(req->method, "HEAD") == 0)
    method = HTTP_METHOD_HEAD;

  Response *res = response_create();
  if (res == NULL) {
    request_destroy(req);
    return NULL;
  }

  http_event_ctx_t ctx = {.response = res};

  esp_http_client_config_t config = {
      .url = url,
      .method = method,
      .event_handler = _http_event_handler,
      .user_data = &ctx,
      .crt_bundle_attach = esp_crt_bundle_attach,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    response_destroy(res);
    request_destroy(req);
    return NULL;
  }

  if (req->headers != NULL) {
    for (size_t i = 0; i < list_size(req->headers); i++) {
      Header *h = list_get(req->headers, i);
      if (h != NULL && h->key != NULL && h->value != NULL) {
        esp_http_client_set_header(client, h->key, h->value);
      }
    }
  }

  if (req->body != NULL && req->body->stream != NULL && req->body->size > 0) {
    esp_http_client_set_post_field(client, req->body->stream, req->body->size);
  }

  esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    res->status_code = esp_http_client_get_status_code(client);
  } else {
    response_destroy(res);
    esp_http_client_cleanup(client);
    request_destroy(req);
    return NULL;
  }

  esp_http_client_cleanup(client);
  request_destroy(req);
  return res;
}
