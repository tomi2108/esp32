#ifndef JSON_H
#define JSON_H

#include "buffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  t_buffer *buffer;
  uint8_t depth;
  bool *first;
  bool *object;
  bool expecting_value;
} t_json;

typedef enum {
  JSON_TYPE_OBJECT,
  JSON_TYPE_ARRAY,
  JSON_TYPE_STRING,
  JSON_TYPE_INT,
  JSON_TYPE_UINT,
  JSON_TYPE_DOUBLE,
  JSON_TYPE_BOOL,
  JSON_TYPE_NULL
} t_json_type;

typedef struct s_json_value t_json_value;

/* Writer */

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

/* Reader */

t_json_value *json_parse(const char *input);
t_json_value *json_parse_buffer(t_buffer *buffer);
void json_value_destroy(t_json_value *value);

t_json_type json_value_type(const t_json_value *value);
const char *json_value_string(const t_json_value *value);

int json_value_int(const t_json_value *value, int32_t *result);
int json_value_uint(const t_json_value *value, uint32_t *result);
int json_value_double(const t_json_value *value, double *result);
int json_value_bool(const t_json_value *value, bool *result);

const t_json_value *json_object_get(
    const t_json_value *object, const char *key);
const t_json_value *json_array_get(
    const t_json_value *array, size_t index);

size_t json_array_size(const t_json_value *array);
size_t json_object_size(const t_json_value *object);

#endif
