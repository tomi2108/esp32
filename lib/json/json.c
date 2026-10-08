#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_MAX_DEPTH 16

static int json_add(t_json *json, const char *data, size_t size) {
  return buffer_add(json->buffer, (void *)data, size);
}

static int json_char(t_json *json, char c) { return json_add(json, &c, 1); }

static int json_prepare_value(t_json *json) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (json->expecting_value) {
    json->expecting_value = false;
    return 0;
  }

  if (!json->first[json->depth - 1]) {
    if (json_char(json, ',') != 0)
      return -1;
  } else {
    json->first[json->depth - 1] = false;
  }

  return 0;
}

static int json_string_raw(t_json *json, const char *value) {
  if (json_char(json, '"') != 0)
    return -1;

  for (size_t i = 0; value[i] != '\0'; i++) {
    char c = value[i];

    switch (c) {
    case '"':
      if (json_add(json, "\\\"", 2) != 0)
        return -1;
      break;

    case '\\':
      if (json_add(json, "\\\\", 2) != 0)
        return -1;
      break;

    case '\b':
      if (json_add(json, "\\b", 2) != 0)
        return -1;
      break;

    case '\f':
      if (json_add(json, "\\f", 2) != 0)
        return -1;
      break;

    case '\n':
      if (json_add(json, "\\n", 2) != 0)
        return -1;
      break;

    case '\r':
      if (json_add(json, "\\r", 2) != 0)
        return -1;
      break;

    case '\t':
      if (json_add(json, "\\t", 2) != 0)
        return -1;
      break;

    default:
      if ((unsigned char)c < 0x20) {
        char escaped[7];

        snprintf(escaped, sizeof(escaped), "\\u%04x", (unsigned char)c);

        if (json_add(json, escaped, 6) != 0)
          return -1;
      } else {
        if (json_char(json, c) != 0)
          return -1;
      }
      break;
    }
  }

  return json_char(json, '"');
}

t_json *json_create(void) {
  t_json *json = malloc(sizeof(t_json));

  if (json == NULL)
    return NULL;

  json->buffer = buffer_create();

  if (json->buffer == NULL) {
    free(json);
    return NULL;
  }

  json->first = calloc(JSON_MAX_DEPTH, sizeof(bool));
  json->object = calloc(JSON_MAX_DEPTH, sizeof(bool));

  if (json->first == NULL || json->object == NULL) {
    free(json->first);
    free(json->object);
    buffer_destroy(json->buffer);
    free(json);
    return NULL;
  }

  json->depth = 0;
  json->expecting_value = false;

  return json;
}

void json_destroy(t_json *json) {
  if (json == NULL)
    return;

  free(json->first);
  free(json->object);
  buffer_destroy(json->buffer);
  free(json);
}

int json_object_start(t_json *json) {
  if (json == NULL || json->depth >= JSON_MAX_DEPTH)
    return -1;

  if (json->depth > 0 && json_prepare_value(json) != 0)
    return -1;

  if (json_char(json, '{') != 0)
    return -1;

  json->first[json->depth] = true;
  json->object[json->depth] = true;
  json->depth++;

  return 0;
}

int json_object_end(t_json *json) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1])
    return -1;

  if (json->expecting_value)
    return -1;

  if (json_char(json, '}') != 0)
    return -1;

  json->depth--;

  return 0;
}

int json_array_start(t_json *json) {
  if (json == NULL || json->depth >= JSON_MAX_DEPTH)
    return -1;

  if (json->depth > 0 && json_prepare_value(json) != 0)
    return -1;

  if (json_char(json, '[') != 0)
    return -1;

  json->first[json->depth] = true;
  json->object[json->depth] = false;
  json->depth++;

  return 0;
}

int json_array_end(t_json *json) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (json->object[json->depth - 1])
    return -1;

  if (json_char(json, ']') != 0)
    return -1;

  json->depth--;

  return 0;
}

int json_key(t_json *json, const char *key) {
  if (json == NULL || key == NULL || json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1])
    return -1;

  if (json->expecting_value)
    return -1;

  if (!json->first[json->depth - 1]) {
    if (json_char(json, ',') != 0)
      return -1;
  } else {
    json->first[json->depth - 1] = false;
  }

  if (json_string_raw(json, key) != 0)
    return -1;

  if (json_char(json, ':') != 0)
    return -1;

  json->expecting_value = true;

  return 0;
}

int json_string(t_json *json, const char *value) {
  if (json == NULL || value == NULL)
    return -1;

  if (json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1] || !json->expecting_value) {
    if (json_prepare_value(json) != 0)
      return -1;
  }

  return json_string_raw(json, value);
}

int json_int(t_json *json, int32_t value) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1] || !json->expecting_value) {
    if (json_prepare_value(json) != 0)
      return -1;
  }

  char number[12];

  int length = snprintf(number, sizeof(number), "%ld", (long)value);

  if (length < 0 || (size_t)length >= sizeof(number))
    return -1;

  return json_add(json, number, (size_t)length);
}

int json_uint(t_json *json, uint32_t value) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1] || !json->expecting_value) {
    if (json_prepare_value(json) != 0)
      return -1;
  }

  char number[11];

  int length = snprintf(number, sizeof(number), "%lu", (unsigned long)value);

  if (length < 0 || (size_t)length >= sizeof(number))
    return -1;

  return json_add(json, number, (size_t)length);
}

int json_bool(t_json *json, bool value) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1] || !json->expecting_value) {
    if (json_prepare_value(json) != 0)
      return -1;
  }

  if (value)
    return json_add(json, "true", 4);

  return json_add(json, "false", 5);
}

int json_null(t_json *json) {
  if (json == NULL || json->depth == 0)
    return -1;

  if (!json->object[json->depth - 1] || !json->expecting_value) {
    if (json_prepare_value(json) != 0)
      return -1;
  }

  return json_add(json, "null", 4);
}

t_buffer *json_buffer(t_json *json) {
  if (json == NULL)
    return NULL;

  return json->buffer;
}
