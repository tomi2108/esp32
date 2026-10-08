#ifndef JSON_H
#define JSON_H

#include "buffer.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  t_buffer *buffer;
  uint8_t depth;
  bool *first;
  bool *object;
  bool expecting_value;
} t_json;

t_json *json_create(void);
void json_destroy(t_json *json);

int json_object_start(t_json *json);
int json_object_end(t_json *json);

int json_array_start(t_json *json);
int json_array_end(t_json *json);

int json_key(t_json *json, const char *key);

int json_string(t_json *json, const char *value);
int json_int(t_json *json, int32_t value);
int json_uint(t_json *json, uint32_t value);
int json_bool(t_json *json, bool value);
int json_null(t_json *json);

t_buffer *json_buffer(t_json *json);

#endif
