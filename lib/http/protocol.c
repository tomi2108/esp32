#include "protocol.h"
#include "buffer.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

Header *header_create(char *key, char *value) {
  Header *h = malloc(sizeof(Header));
  if (h == NULL)
    return NULL;

  h->key = malloc(1 + strlen(key));
  h->value = malloc(1 + strlen(value));

  if (h->key == NULL || h->value == NULL) {
    free(h->key);
    free(h->value);
    free(h);
    return NULL;
  }

  strcpy(h->key, key);
  strcpy(h->value, value);
  return h;
}

void header_destroy(Header *h) {
  if (h == NULL)
    return;
  free(h->key);
  free(h->value);
  free(h);
}

Headers headers_create() {
  return list_create();
}

void headers_destroy(Headers headers) {
  if (headers == NULL)
    return;
  list_destroy_and_destroy_elements(headers, (void (*)(void *))&header_destroy);
}

void headers_add(Headers h, char *key, char *value) {
  if (h == NULL || key == NULL || value == NULL)
    return;
  Header *header = header_create(key, value);
  if (header != NULL) {
    list_add(h, header);
  }
}

char *headers_get(Headers h, char *key) {
  if (h == NULL || key == NULL)
    return NULL;

  for (size_t i = 0; i < list_size(h); i++) {
    Header *header = list_get(h, i);
    if (header != NULL && strcmp(key, header->key) == 0) {
      return header->value;
    }
  }
  return NULL;
}

Body body_create() {
  return buffer_create();
}

void body_destroy(Body body) {
  buffer_destroy(body);
}

void body_add_string(Body body, char *string, uint32_t length) {
  if (body != NULL && string != NULL) {
    buffer_add_string(body, length, string);
  }
}
