
#include "client.h"

#include "esp_heap_caps.h"
#include "freertos/semphr.h"

#define TAG "HTTP"
#define HTTP_URL_MAX 256

typedef struct {
  Response *response;
  size_t max_body_size;
} HttpEventContext;

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
  HttpEventContext *ctx = evt->user_data;

  if (evt->event_id == HTTP_EVENT_ON_DATA) {
    if (ctx == NULL || ctx->response == NULL || ctx->response->body == NULL ||
        evt->data_len < 0 || (evt->data_len > 0 && evt->data == NULL)) {
      return ESP_FAIL;
    }

    size_t length = (size_t)evt->data_len;
    size_t current = ctx->response->body->size;

    if (current > ctx->max_body_size || length > ctx->max_body_size - current) {
      ESP_LOGE(TAG, "Response body exceeds %u bytes",
               (unsigned)ctx->max_body_size);
      return ESP_FAIL;
    }

    if (length > 0 && buffer_add(ctx->response->body, evt->data, length) != 0) {
      ESP_LOGE(TAG, "Failed to allocate response body");
      return ESP_FAIL;
    }
  }

  if (evt->event_id == HTTP_EVENT_ERROR) {
    ESP_LOGE(TAG, "HTTP transport error");
  }

  return ESP_OK;
}

HttpClient *client_create(const char *baseUrl) {
  if (baseUrl == NULL || baseUrl[0] == '\0')
    return NULL;

  HttpClient *client = calloc(1, sizeof(*client));
  if (client == NULL)
    return NULL;

  client->baseUrl = baseUrl;
  client->headers = headers_create();
  client->mutex = xSemaphoreCreateMutex();

  if (client->headers == NULL || client->mutex == NULL) {
    if (client->headers != NULL)
      headers_destroy(client->headers);
    if (client->mutex != NULL)
      vSemaphoreDelete(client->mutex);
    free(client);
    return NULL;
  }

  headers_add(client->headers, "Connection", "close");
  headers_add(client->headers, "Accept-Encoding", "identity");

  return client;
}

void client_destroy(HttpClient *client) {
  if (client == NULL)
    return;

  if (client->headers != NULL)
    headers_destroy(client->headers);

  if (client->mutex != NULL)
    vSemaphoreDelete(client->mutex);

  free(client);
}

static esp_http_client_method_t get_method(const char *method) {
  if (strcmp(method, "POST") == 0)
    return HTTP_METHOD_POST;
  if (strcmp(method, "PUT") == 0)
    return HTTP_METHOD_PUT;
  if (strcmp(method, "PATCH") == 0)
    return HTTP_METHOD_PATCH;
  if (strcmp(method, "DELETE") == 0)
    return HTTP_METHOD_DELETE;
  if (strcmp(method, "OPTIONS") == 0)
    return HTTP_METHOD_OPTIONS;
  if (strcmp(method, "HEAD") == 0)
    return HTTP_METHOD_HEAD;

  return HTTP_METHOD_GET;
}

Response *client_send(HttpClient *client, const char *method, const char *url,
                      Body body) {
  if (client == NULL || client->baseUrl == NULL || method == NULL ||
      url == NULL || url[0] == '\0') {
    return NULL;
  }

  if (xSemaphoreTake(client->mutex, portMAX_DELAY) != pdTRUE)
    return NULL;

  Response *res = NULL;
  esp_http_client_handle_t handle = NULL;
  char full_url[HTTP_URL_MAX];

  int written;

  if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
    written = snprintf(full_url, sizeof(full_url), "%s", url);
  } else {
    size_t base_len = strlen(client->baseUrl);

    bool need_slash = client->baseUrl[base_len - 1] != '/' && url[0] != '/';

    written = snprintf(full_url, sizeof(full_url), "%s%s%s", client->baseUrl,
                       need_slash ? "/" : "", url);
  }

  if (written < 0 || (size_t)written >= sizeof(full_url)) {
    ESP_LOGE(TAG, "Request URL too long");
    goto cleanup;
  }

  res = response_create();
  if (res == NULL) {
    ESP_LOGE(TAG, "Failed to allocate response");
    goto cleanup;
  }

  HttpEventContext ctx = {
      .response = res,
      .max_body_size = 16384,
  };

  esp_http_client_config_t config = {
      .url = full_url,
      .method = get_method(method),
      .event_handler = http_event_handler,
      .user_data = &ctx,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .timeout_ms = 15000,
  };

  handle = esp_http_client_init(&config);
  if (handle == NULL) {
    ESP_LOGE(TAG, "Failed to initialize HTTP client");
    goto cleanup;
  }

  for (size_t i = 0; i < list_size(client->headers); i++) {
    Header *header = list_get(client->headers, i);

    if (header != NULL && header->key != NULL && header->value != NULL) {
      esp_err_t err =
          esp_http_client_set_header(handle, header->key, header->value);

      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP header: %s", header->key);
        goto cleanup;
      }
    }
  }

  if (body != NULL && body->stream != NULL && body->size > 0) {
    esp_err_t err = esp_http_client_set_post_field(
        handle, (const char *)body->stream, (int)body->size);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set HTTP request body");
      goto cleanup;
    }
  }

  ESP_LOGI(TAG, "%s %s; free heap=%lu, largest block=%lu", method, full_url,
           (unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT),
           (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

  esp_err_t err = esp_http_client_perform(handle);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "%s %s failed: %s (0x%x)", method, full_url,
             esp_err_to_name(err), (unsigned int)err);
    goto cleanup;
  }

  res->status_code = esp_http_client_get_status_code(handle);

  ESP_LOGI(TAG, "%s %s -> %u, body=%u bytes", method, full_url,
           (unsigned)res->status_code, (unsigned)res->body->size);

  goto done;

cleanup:
  if (res != NULL) {
    response_destroy(res);
    res = NULL;
  }

done:
  if (handle != NULL)
    esp_http_client_cleanup(handle);

  xSemaphoreGive(client->mutex);
  return res;
}

Response *client_send_get(HttpClient *client, const char *url) {
  return client_send(client, "GET", url, NULL);
}

Response *client_send_post(HttpClient *client, Body body, const char *url) {
  return client_send(client, "POST", url, body);
}

Response *client_send_put(HttpClient *client, const char *url) {
  return client_send(client, "PUT", url, NULL);
}

Response *client_send_patch(HttpClient *client, const char *url) {
  return client_send(client, "PATCH", url, NULL);
}

Response *client_send_delete(HttpClient *client, const char *url) {
  return client_send(client, "DELETE", url, NULL);
}

Response *client_send_options(HttpClient *client, const char *url) {
  return client_send(client, "OPTIONS", url, NULL);
}

Response *client_send_head(HttpClient *client, const char *url) {
  return client_send(client, "HEAD", url, NULL);
}
