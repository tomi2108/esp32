#include "server.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static esp_err_t _universal_http_handler(httpd_req_t *req) {
  HttpServer *server = (HttpServer *)req->user_ctx;

  Request *custom_req = request_create();
  if (!custom_req)
    return ESP_FAIL;

  snprintf(custom_req->method, sizeof(custom_req->method), "%s",
           req->method == HTTP_GET      ? "GET"
           : req->method == HTTP_POST   ? "POST"
           : req->method == HTTP_PUT    ? "PUT"
           : req->method == HTTP_DELETE ? "DELETE"
           : req->method == HTTP_PATCH  ? "PATCH"
                                        : "GET");

  snprintf(custom_req->path, sizeof(custom_req->path), "%s", req->uri);

  if (req->content_len > 0) {
    char *content = malloc(req->content_len + 1);
    if (content) {
      int ret = httpd_req_recv(req, content, req->content_len);
      if (ret > 0) {
        content[ret] = '\0';
        body_add_string(custom_req->body, content, ret);
      }
      free(content);
    }
  }

  Response *custom_res = response_create();
  if (!custom_res) {
    request_destroy(custom_req);
    return ESP_FAIL;
  }
  custom_res->status_code = 200;

  Handler matched_handler = NULL;
  for (size_t i = 0; i < list_size(server->endpoints); i++) {
    Endpoint *endpoint = list_get(server->endpoints, i);
    if (strcmp(endpoint->path, custom_req->path) == 0 &&
        strcmp(endpoint->method, custom_req->method) == 0) {
      matched_handler = endpoint->handler;
      break;
    }
  }

  if (matched_handler) {
    matched_handler(custom_req, custom_res);
  } else {
    custom_res->status_code = 404;
  }

  httpd_resp_set_status(req, status[custom_res->status_code]
                                 ? status[custom_res->status_code]
                                 : "200 OK");

  if (custom_res->body && custom_res->body->size > 0) {
    httpd_resp_send(req, custom_res->body->stream, custom_res->body->size);
  } else {
    httpd_resp_send(req, "", 0);
  }

  request_destroy(custom_req);
  response_destroy(custom_res);
  return ESP_OK;
}

HttpServer *server_create() {
  HttpServer *server = malloc(sizeof(HttpServer));
  if (server == NULL)
    return NULL;

  server->handle = NULL;
  server->endpoints = list_create();
  return server;
}

void server_endpoint(HttpServer *server, Method method, Path path,
                     Handler handler) {
  Endpoint *endpoint = malloc(sizeof(Endpoint));
  if (endpoint == NULL)
    return;

  endpoint->method = method;
  endpoint->path = path;
  endpoint->handler = handler;

  httpd_method_t esp_method = HTTP_GET;
  if (strcmp(method, "POST") == 0)
    esp_method = HTTP_POST;
  else if (strcmp(method, "PUT") == 0)
    esp_method = HTTP_PUT;
  else if (strcmp(method, "DELETE") == 0)
    esp_method = HTTP_DELETE;
  else if (strcmp(method, "PATCH") == 0)
    esp_method = HTTP_PATCH;

  endpoint->uri_cfg = (httpd_uri_t){.uri = path,
                                    .method = esp_method,
                                    .handler = _universal_http_handler,
                                    .user_ctx = server};

  list_add(server->endpoints, endpoint);

  if (server->handle != NULL) {
    httpd_register_uri_handler(server->handle, &endpoint->uri_cfg);
  }
}

int server_listen(HttpServer *server, char *port, void (*callback)()) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = atoi(port);

  if (httpd_start(&server->handle, &config) == ESP_OK) {
    for (size_t i = 0; i < list_size(server->endpoints); i++) {
      Endpoint *endpoint = list_get(server->endpoints, i);
      httpd_register_uri_handler(server->handle, &endpoint->uri_cfg);
    }

    if (callback) {
      callback();
    }
    return 0;
  }

  return -1;
}

void server_get(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "GET", path, handler);
}

void server_post(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "POST", path, handler);
}

void server_put(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "PUT", path, handler);
}

void server_patch(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "PATCH", path, handler);
}

void server_delete(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "DELETE", path, handler);
}

void server_options(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "OPTIONS", path, handler);
}

void server_head(HttpServer *server, Path path, Handler handler) {
  server_endpoint(server, "HEAD", path, handler);
}
