#include "client.h"

typedef struct {
  Response *response;
} http_event_ctx_t;

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  http_event_ctx_t *ctx = (http_event_ctx_t *)evt->user_data;
  switch (evt->event_id) {
  case HTTP_EVENT_ON_CONNECTED:
    break;
  case HTTP_EVENT_HEADER_SENT:
    break;
  case HTTP_EVENT_ON_HEADER:
    if (ctx && ctx->response && ctx->response->headers) {
      headers_add(ctx->response->headers, evt->header_key, evt->header_value);
    }
    break;
  case HTTP_EVENT_ON_DATA:
    if (ctx && ctx->response && ctx->response->body) {
      buffer_add(ctx->response->body, evt->data, evt->data_len);
    };
    break;
  case HTTP_EVENT_ON_FINISH:
    break;
  case HTTP_EVENT_DISCONNECTED:
    break;
  default:
    break;
  }
  return ESP_OK;
}

HttpClient *client_create(char *baseUrl) {
  HttpClient *client = malloc(sizeof(HttpClient));
  if (!client)
    return NULL;

  client->baseUrl = baseUrl;
  client->headers = headers_create();

  char host[100] = {0};
  if (sscanf(baseUrl, "http://%99[^:/]", host) == 1 ||
      sscanf(baseUrl, "https://%99[^:/]", host) == 1) {
    headers_add(client->headers, "Host", host);
  }

  headers_add(client->headers, "Connection", "close");
  headers_add(client->headers, "Accept-Encoding", "identity");
  return client;
}

void client_destroy(HttpClient *client) {
  if (!client)
    return;
  if (client->headers)
    headers_destroy(client->headers);
  free(client);
}

Response *client_send(HttpClient *client, Method method, char *url, Body body) {
  char full_url[256] = {0};

  if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
    snprintf(full_url, sizeof(full_url), "%s", url);
  } else {
    int need_slash =
        (client->baseUrl[strlen(client->baseUrl) - 1] != '/' && url[0] != '/');
    snprintf(full_url, sizeof(full_url), "%s%s%s", client->baseUrl,
             need_slash ? "/" : "", url);
  }

  esp_http_client_method_t esp_method = HTTP_METHOD_GET;
  if (strcmp(method, "POST") == 0)
    esp_method = HTTP_METHOD_POST;
  else if (strcmp(method, "PUT") == 0)
    esp_method = HTTP_METHOD_PUT;
  else if (strcmp(method, "PATCH") == 0)
    esp_method = HTTP_METHOD_PATCH;
  else if (strcmp(method, "DELETE") == 0)
    esp_method = HTTP_METHOD_DELETE;
  else if (strcmp(method, "OPTIONS") == 0)
    esp_method = HTTP_METHOD_OPTIONS;
  else if (strcmp(method, "HEAD") == 0)
    esp_method = HTTP_METHOD_HEAD;

  Response *res = response_create();
  if (!res)
    return NULL;

  http_event_ctx_t ctx = {.response = res};

  esp_http_client_config_t config = {
      .url = full_url,
      .method = esp_method,
      .event_handler = _http_event_handler,
      .user_data = &ctx,
      .crt_bundle_attach = esp_crt_bundle_attach,
  };

  esp_http_client_handle_t client_handle = esp_http_client_init(&config);
  if (client_handle == NULL) {
    response_destroy(res);
    return NULL;
  }

  if (body && body->stream && body->size > 0) {
    esp_http_client_set_post_field(client_handle, body->stream, body->size);
  }

  esp_err_t err = esp_http_client_perform(client_handle);
  if (err == ESP_OK) {
    res->status_code = esp_http_client_get_status_code(client_handle);
  } else {
    response_destroy(res);
    esp_http_client_cleanup(client_handle);
    return NULL;
  }

  esp_http_client_cleanup(client_handle);
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
