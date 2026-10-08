#include "protocol.h"
#include "buffer.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

Header *header_create(char *key, char *value) {
  Header *h = malloc(sizeof(Header));
  if (h == NULL)
    return NULL;

  h->key = malloc(1 + strlen(key) * sizeof(char));
  h->value = malloc(1 + strlen(value) * sizeof(char));

  memset(h->key, 0, 1 + strlen(key));
  memset(h->value, 0, 1 + strlen(value));

  memcpy(h->key, key, strlen(key));
  memcpy(h->value, value, strlen(value));
  return h;
}

void header_destroy(Header *h) {
  free(h->key);
  free(h->value);
  free(h);
}

Headers headers_create() { return list_create(); }
void headers_destroy(Headers headers) {
  list_destroy_and_destroy_elements(headers, (void (*)(void *))&header_destroy);
}

void headers_add(Headers h, char *key, char *value) {
  // TODO: handle duplicates
  Header *header = header_create(key, value);
  list_add(h, header);
}

char *headers_get(Headers h, char *key) {
  auto bool closure(void *arg) {
    Header *header = (Header *)arg;
    return strcmp(key, header->key) == 0;
  };
  Header *found = list_find(h, &closure);
  if (found == NULL)
    return NULL;
  return found->value;
}

Body body_create() { return buffer_create(); }
void body_destroy(Body body) { return buffer_destroy(body); }
void body_add_string(Body body, char *string, uint32_t length) {
  buffer_add_string(body, length, string);
}

char *status[600] = {
    [100] = "Continue",
    [101] = "Switching Protocols",
    [102] = "Processing",
    [103] = "Early Hints",

    [200] = "OK",
    [201] = "Created",
    [202] = "Accepted",
    [203] = "Non-Authoritative Information",
    [204] = "No content",
    [205] = "Reset Content",
    [206] = "Partial Content",
    [207] = "Multi-Status",
    [208] = "Already Reported",
    [214] = "Transformation Applied",
    [226] = "IM Used",

    [300] = "Multiple Choices",
    [301] = "Moved Permanently",
    [302] = "Found",
    [303] = "See Other",
    [304] = "Not Modified",
    [305] = "Use Proxy",
    [307] = "Temporary Redirect",
    [308] = "Permanent Redirect",

    [400] = "Bad Request",
    [401] = "Unauthorized",
    [402] = "Payment Required",
    [403] = "Forbidden",
    [404] = "Not Found",
    [405] = "Method Not Allowed",
    [406] = "Not Acceptable",
    [407] = "Proxy Authentication Required",
    [408] = "Request Timeout",
    [409] = "Conflict",
    [410] = "Gone",
    [411] = "Length Required",
    [412] = "Precondition Failed",
    [413] = "Content Too Large",
    [414] = "URI Too Long",
    [415] = "Unsupported Media Type",
    [416] = "Range Not Satisfiable",
    [417] = "Expectation Failed",
    [418] = "I'm a teapot",
    [419] = "Page Expired",
    [420] = "Enhance your calm",
    [421] = "Misdirected Request",
    [422] = "Unprocessable Content",
    [423] = "Locked",
    [424] = "Failed Dependency",
    [425] = "Too Early",
    [426] = "Upgrade Required",
    [428] = "Precondition Required",
    [429] = "Too Many Requests",
    [431] = "Request Header Fields Too Large",
    [451] = "Unavailable For Legal Reasons ",

    [500] = "Internal Server Error",
    [501] = "Not Implemented",
    [502] = "Bad Gateway",
    [503] = "Service Unavailable ",
    [504] = "Gateway Timeout",
    [506] = "Variant Also Negotiates",
    [507] = "Insufficient Storage",
    [508] = "Loop Detected",
    [510] = "Not Extended",
    [511] = "Network Authentication Required",
    [599] = "Network Connect Timeout Error",
};
