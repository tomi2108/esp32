#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "buffer.h"
#include "list.h"
#include <string.h>
#include <stdint.h>

typedef char *Method;
typedef char *Path;
typedef t_buffer *Body;

typedef struct Header {
  char *key;
  char *value;
} Header;

typedef t_list *Headers;

Headers headers_create();
void headers_destroy(Headers headers);
void headers_add(Headers h, char *key, char *value);
char *headers_get(Headers h, char *key);

Body body_create();
void body_destroy(Body body);
void body_add_string(Body body, char *string, uint32_t length);

#endif
