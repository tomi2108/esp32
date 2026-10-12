#include "json.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_MAX_DEPTH 16

typedef struct {
  char *key;
  t_json_value *value;
} t_json_member;

struct s_json_value {
  t_json_type type;

  union {
    char *string;
    int32_t int_value;
    uint32_t uint_value;
    double double_value;
    bool bool_value;

    struct {
      t_json_value **items;
      size_t size;
      size_t capacity;
    } array;

    struct {
      t_json_member *items;
      size_t size;
      size_t capacity;
    } object;
  } data;
};

typedef struct {
  const char *input;
  size_t position;
  unsigned int depth;
} t_json_parser;

/* Writer */

static int json_add(t_json *json, const char *data, size_t size) {
  if (json == NULL || json->buffer == NULL || data == NULL)
    return -1;

  return buffer_add(json->buffer, (void *)data, size);
}

static int json_char(t_json *json, char c) { return json_add(json, &c, 1); }

static int json_prepare_value(t_json *json) {
  if (json == NULL || json->depth == 0)
    return -1;

  size_t index = json->depth - 1;

  if (json->object[index]) {
    if (!json->expecting_value)
      return -1;

    json->expecting_value = false;
    return 0;
  }

  if (!json->first[index]) {
    if (json_char(json, ',') != 0)
      return -1;
  } else {
    json->first[index] = false;
  }

  return 0;
}

static int json_string_raw(t_json *json, const char *value) {
  if (value == NULL || json_char(json, '"') != 0)
    return -1;

  for (size_t i = 0; value[i] != '\0'; i++) {
    unsigned char c = (unsigned char)value[i];

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
      if (c < 0x20) {
        char escaped[7];

        snprintf(escaped, sizeof(escaped), "\\u%04x", c);

        if (json_add(json, escaped, 6) != 0)
          return -1;
      } else if (json_char(json, (char)c) != 0) {
        return -1;
      }
      break;
    }
  }

  return json_char(json, '"');
}

t_json *json_create(void) {
  t_json *json = malloc(sizeof(*json));

  if (json == NULL)
    return NULL;

  json->buffer = buffer_create();
  json->first = calloc(JSON_MAX_DEPTH, sizeof(bool));
  json->object = calloc(JSON_MAX_DEPTH, sizeof(bool));
  json->depth = 0;
  json->expecting_value = false;

  if (json->buffer == NULL || json->first == NULL || json->object == NULL) {
    free(json->first);
    free(json->object);
    buffer_destroy(json->buffer);
    free(json);
    return NULL;
  }

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

  size_t index = json->depth - 1;

  if (!json->object[index] || json->expecting_value)
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

  size_t index = json->depth - 1;

  if (!json->object[index] || json->expecting_value)
    return -1;

  if (!json->first[index]) {
    if (json_char(json, ',') != 0)
      return -1;
  } else {
    json->first[index] = false;
  }

  if (json_string_raw(json, key) != 0 || json_char(json, ':') != 0)
    return -1;

  json->expecting_value = true;
  return 0;
}

int json_string(t_json *json, const char *value) {
  if (json == NULL || value == NULL || json->depth == 0)
    return -1;

  if (json_prepare_value(json) != 0)
    return -1;

  return json_string_raw(json, value);
}

int json_int(t_json *json, int32_t value) {
  if (json == NULL || json->depth == 0 || json_prepare_value(json) != 0)
    return -1;

  char number[12];
  int length = snprintf(number, sizeof(number), "%ld", (long)value);

  if (length < 0 || (size_t)length >= sizeof(number))
    return -1;

  return json_add(json, number, (size_t)length);
}

int json_uint(t_json *json, uint32_t value) {
  if (json == NULL || json->depth == 0 || json_prepare_value(json) != 0)
    return -1;

  char number[11];
  int length = snprintf(number, sizeof(number), "%lu", (unsigned long)value);

  if (length < 0 || (size_t)length >= sizeof(number))
    return -1;

  return json_add(json, number, (size_t)length);
}

int json_bool(t_json *json, bool value) {
  if (json == NULL || json->depth == 0 || json_prepare_value(json) != 0)
    return -1;

  return value ? json_add(json, "true", 4) : json_add(json, "false", 5);
}

int json_null(t_json *json) {
  if (json == NULL || json->depth == 0 || json_prepare_value(json) != 0)
    return -1;

  return json_add(json, "null", 4);
}

t_buffer *json_buffer(t_json *json) {
  return json != NULL ? json->buffer : NULL;
}

/* Reader: value allocation and destruction */

static t_json_value *json_value_create(t_json_type type) {
  t_json_value *value = calloc(1, sizeof(*value));

  if (value != NULL)
    value->type = type;

  return value;
}

void json_value_destroy(t_json_value *value) {
  if (value == NULL)
    return;

  if (value->type == JSON_TYPE_STRING) {
    free(value->data.string);
  } else if (value->type == JSON_TYPE_ARRAY) {
    for (size_t i = 0; i < value->data.array.size; i++)
      json_value_destroy(value->data.array.items[i]);

    free(value->data.array.items);
  } else if (value->type == JSON_TYPE_OBJECT) {
    for (size_t i = 0; i < value->data.object.size; i++) {
      free(value->data.object.items[i].key);
      json_value_destroy(value->data.object.items[i].value);
    }

    free(value->data.object.items);
  }

  free(value);
}

static int json_array_append(t_json_value *array, t_json_value *item) {
  if (array->data.array.size == array->data.array.capacity) {
    size_t capacity =
        array->data.array.capacity == 0 ? 4 : array->data.array.capacity * 2;

    if (capacity < array->data.array.capacity ||
        capacity > SIZE_MAX / sizeof(*array->data.array.items))
      return -1;

    t_json_value **items =
        realloc(array->data.array.items, capacity * sizeof(*items));

    if (items == NULL)
      return -1;

    array->data.array.items = items;
    array->data.array.capacity = capacity;
  }

  array->data.array.items[array->data.array.size++] = item;
  return 0;
}

static int json_object_append(t_json_value *object, char *key,
                              t_json_value *item) {
  if (object->data.object.size == object->data.object.capacity) {
    size_t capacity = object->data.object.capacity == 0
                          ? 4
                          : object->data.object.capacity * 2;

    if (capacity < object->data.object.capacity ||
        capacity > SIZE_MAX / sizeof(*object->data.object.items))
      return -1;

    t_json_member *items =
        realloc(object->data.object.items, capacity * sizeof(*items));

    if (items == NULL)
      return -1;

    object->data.object.items = items;
    object->data.object.capacity = capacity;
  }

  t_json_member *member =
      &object->data.object.items[object->data.object.size++];

  member->key = key;
  member->value = item;
  return 0;
}

/* Reader: parser helpers */

static char json_peek(const t_json_parser *parser) {
  return parser->input[parser->position];
}

static bool json_consume(t_json_parser *parser, char expected) {
  if (json_peek(parser) != expected)
    return false;

  parser->position++;
  return true;
}

static void json_skip_whitespace(t_json_parser *parser) {
  while (json_peek(parser) == ' ' || json_peek(parser) == '\t' ||
         json_peek(parser) == '\n' || json_peek(parser) == '\r')
    parser->position++;
}

static int json_hex_value(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return -1;
}

static int json_read_hex4(t_json_parser *parser, uint32_t *result) {
  uint32_t value = 0;

  for (int i = 0; i < 4; i++) {
    int digit = json_hex_value(json_peek(parser));

    if (digit < 0)
      return -1;

    parser->position++;
    value = (value << 4) | (uint32_t)digit;
  }

  *result = value;
  return 0;
}

static int json_append_byte(char **buffer, size_t *length, size_t *capacity,
                            unsigned char byte) {
  if (*length > SIZE_MAX - 2)
    return -1;

  size_t needed = *length + 2;

  if (needed > *capacity) {
    size_t next = *capacity == 0 ? 16 : *capacity;

    while (next < needed) {
      if (next > SIZE_MAX / 2) {
        next = needed;
        break;
      }

      next *= 2;
    }

    char *resized = realloc(*buffer, next);

    if (resized == NULL)
      return -1;

    *buffer = resized;
    *capacity = next;
  }

  (*buffer)[(*length)++] = (char)byte;
  (*buffer)[*length] = '\0';
  return 0;
}

static int json_append_utf8(char **buffer, size_t *length, size_t *capacity,
                            uint32_t cp) {
  if (cp <= 0x7F)
    return json_append_byte(buffer, length, capacity, (unsigned char)cp);

  if (cp <= 0x7FF) {
    if (json_append_byte(buffer, length, capacity,
                         (unsigned char)(0xC0 | (cp >> 6))) != 0)
      return -1;

    return json_append_byte(buffer, length, capacity,
                            (unsigned char)(0x80 | (cp & 0x3F)));
  }

  if (cp <= 0xFFFF) {
    if (json_append_byte(buffer, length, capacity,
                         (unsigned char)(0xE0 | (cp >> 12))) != 0 ||
        json_append_byte(buffer, length, capacity,
                         (unsigned char)(0x80 | ((cp >> 6) & 0x3F))) != 0)
      return -1;

    return json_append_byte(buffer, length, capacity,
                            (unsigned char)(0x80 | (cp & 0x3F)));
  }

  if (cp <= 0x10FFFF) {
    if (json_append_byte(buffer, length, capacity,
                         (unsigned char)(0xF0 | (cp >> 18))) != 0 ||
        json_append_byte(buffer, length, capacity,
                         (unsigned char)(0x80 | ((cp >> 12) & 0x3F))) != 0 ||
        json_append_byte(buffer, length, capacity,
                         (unsigned char)(0x80 | ((cp >> 6) & 0x3F))) != 0)
      return -1;

    return json_append_byte(buffer, length, capacity,
                            (unsigned char)(0x80 | (cp & 0x3F)));
  }

  return -1;
}

/* Reader: JSON strings */

static char *json_parse_string_raw(t_json_parser *parser) {
  if (!json_consume(parser, '"'))
    return NULL;

  char *buffer = NULL;
  size_t length = 0;
  size_t capacity = 0;

  while (json_peek(parser) != '\0' && json_peek(parser) != '"') {
    unsigned char c = (unsigned char)json_peek(parser);
    parser->position++;

    if (c < 0x20)
      goto error;

    if (c != '\\') {
      if (json_append_byte(&buffer, &length, &capacity, c) != 0)
        goto error;

      continue;
    }

    char escape = json_peek(parser);

    if (escape == '\0')
      goto error;

    parser->position++;

    unsigned char decoded;

    switch (escape) {
    case '"':
      decoded = '"';
      break;
    case '\\':
      decoded = '\\';
      break;
    case '/':
      decoded = '/';
      break;
    case 'b':
      decoded = '\b';
      break;
    case 'f':
      decoded = '\f';
      break;
    case 'n':
      decoded = '\n';
      break;
    case 'r':
      decoded = '\r';
      break;
    case 't':
      decoded = '\t';
      break;

    case 'u': {
      uint32_t cp;

      if (json_read_hex4(parser, &cp) != 0)
        goto error;

      if (cp >= 0xD800 && cp <= 0xDBFF) {
        uint32_t low;

        if (!json_consume(parser, '\\') || !json_consume(parser, 'u') ||
            json_read_hex4(parser, &low) != 0 || low < 0xDC00 || low > 0xDFFF)
          goto error;

        cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
      } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
        goto error;
      }

      if (json_append_utf8(&buffer, &length, &capacity, cp) != 0)
        goto error;

      continue;
    }

    default:
      goto error;
    }

    if (json_append_byte(&buffer, &length, &capacity, decoded) != 0)
      goto error;
  }

  if (!json_consume(parser, '"'))
    goto error;

  if (buffer == NULL) {
    buffer = malloc(1);

    if (buffer == NULL)
      return NULL;

    buffer[0] = '\0';
  }

  return buffer;

error:
  free(buffer);
  return NULL;
}

/* Reader: objects and arrays */

static t_json_value *json_parse_value(t_json_parser *parser);

static t_json_value *json_parse_object(t_json_parser *parser) {
  if (!json_consume(parser, '{'))
    return NULL;

  t_json_value *object = json_value_create(JSON_TYPE_OBJECT);

  if (object == NULL)
    return NULL;

  json_skip_whitespace(parser);

  if (json_consume(parser, '}'))
    return object;

  while (true) {
    if (json_peek(parser) != '"')
      goto error;

    char *key = json_parse_string_raw(parser);

    if (key == NULL)
      goto error;

    json_skip_whitespace(parser);

    if (!json_consume(parser, ':')) {
      free(key);
      goto error;
    }

    json_skip_whitespace(parser);

    t_json_value *value = json_parse_value(parser);

    if (value == NULL) {
      free(key);
      goto error;
    }

    if (json_object_append(object, key, value) != 0) {
      free(key);
      json_value_destroy(value);
      goto error;
    }

    json_skip_whitespace(parser);

    if (json_consume(parser, '}'))
      return object;

    if (!json_consume(parser, ','))
      goto error;

    json_skip_whitespace(parser);
  }

error:
  json_value_destroy(object);
  return NULL;
}

static t_json_value *json_parse_array(t_json_parser *parser) {
  if (!json_consume(parser, '['))
    return NULL;

  t_json_value *array = json_value_create(JSON_TYPE_ARRAY);

  if (array == NULL)
    return NULL;

  json_skip_whitespace(parser);

  if (json_consume(parser, ']'))
    return array;

  while (true) {
    t_json_value *value = json_parse_value(parser);

    if (value == NULL)
      goto error;

    if (json_array_append(array, value) != 0) {
      json_value_destroy(value);
      goto error;
    }

    json_skip_whitespace(parser);

    if (json_consume(parser, ']'))
      return array;

    if (!json_consume(parser, ','))
      goto error;

    json_skip_whitespace(parser);
  }

error:
  json_value_destroy(array);
  return NULL;
}

/* Reader: numbers and literals */

static t_json_value *json_parse_number(t_json_parser *parser) {
  const char *start = parser->input + parser->position;
  const char *p = start;

  if (*p == '-')
    p++;

  if (*p == '0') {
    p++;

    if (isdigit((unsigned char)*p))
      return NULL;
  } else {
    if (*p < '1' || *p > '9')
      return NULL;

    while (isdigit((unsigned char)*p))
      p++;
  }

  bool is_double = false;

  if (*p == '.') {
    is_double = true;
    p++;

    if (!isdigit((unsigned char)*p))
      return NULL;

    while (isdigit((unsigned char)*p))
      p++;
  }

  if (*p == 'e' || *p == 'E') {
    is_double = true;
    p++;

    if (*p == '+' || *p == '-')
      p++;

    if (!isdigit((unsigned char)*p))
      return NULL;

    while (isdigit((unsigned char)*p))
      p++;
  }

  size_t length = (size_t)(p - start);

  if (length == 0 || length == SIZE_MAX)
    return NULL;

  char *text = malloc(length + 1);

  if (text == NULL)
    return NULL;

  memcpy(text, start, length);
  text[length] = '\0';

  char *end = NULL;
  errno = 0;

  t_json_value *value = NULL;

  if (is_double) {
    double number = strtod(text, &end);

    if (errno == ERANGE || end != text + length || !isfinite(number))
      goto done;

    value = json_value_create(JSON_TYPE_DOUBLE);

    if (value != NULL)
      value->data.double_value = number;
  } else if (text[0] == '-') {
    long long number = strtoll(text, &end, 10);

    if (errno == ERANGE || end != text + length || number < INT32_MIN ||
        number > INT32_MAX)
      goto done;

    value = json_value_create(JSON_TYPE_INT);

    if (value != NULL)
      value->data.int_value = (int32_t)number;
  } else {
    unsigned long long number = strtoull(text, &end, 10);

    if (errno == ERANGE || end != text + length || number > UINT32_MAX)
      goto done;

    value = json_value_create(JSON_TYPE_UINT);

    if (value != NULL)
      value->data.uint_value = (uint32_t)number;
  }

done:
  free(text);

  if (value != NULL)
    parser->position += length;

  return value;
}

static bool json_match_literal(t_json_parser *parser, const char *literal) {
  size_t length = strlen(literal);

  if (strncmp(parser->input + parser->position, literal, length) != 0)
    return false;

  parser->position += length;
  return true;
}

static t_json_value *json_parse_value(t_json_parser *parser) {
  json_skip_whitespace(parser);

  if (parser->depth >= JSON_MAX_DEPTH)
    return NULL;

  char c = json_peek(parser);

  if (c == '{' || c == '[') {
    parser->depth++;

    t_json_value *value =
        c == '{' ? json_parse_object(parser) : json_parse_array(parser);

    parser->depth--;
    return value;
  }

  if (c == '"') {
    char *string = json_parse_string_raw(parser);

    if (string == NULL)
      return NULL;

    t_json_value *value = json_value_create(JSON_TYPE_STRING);

    if (value == NULL) {
      free(string);
      return NULL;
    }

    value->data.string = string;
    return value;
  }

  if (c == 't' || c == 'f') {
    bool boolean;

    if (c == 't' && json_match_literal(parser, "true")) {
      boolean = true;
    } else if (c == 'f' && json_match_literal(parser, "false")) {
      boolean = false;
    } else {
      return NULL;
    }

    t_json_value *value = json_value_create(JSON_TYPE_BOOL);

    if (value != NULL)
      value->data.bool_value = boolean;

    return value;
  }

  if (c == 'n') {
    if (!json_match_literal(parser, "null"))
      return NULL;

    return json_value_create(JSON_TYPE_NULL);
  }

  if (c == '-' || (c >= '0' && c <= '9'))
    return json_parse_number(parser);

  return NULL;
}

/* Reader: public parsing functions */

t_json_value *json_parse(const char *input) {
  if (input == NULL)
    return NULL;

  t_json_parser parser = {
      .input = input,
      .position = 0,
      .depth = 0,
  };

  json_skip_whitespace(&parser);

  t_json_value *root = json_parse_value(&parser);

  if (root == NULL)
    return NULL;

  json_skip_whitespace(&parser);

  if (json_peek(&parser) != '\0') {
    json_value_destroy(root);
    return NULL;
  }

  return root;
}

t_json_value *json_parse_buffer(t_buffer *buffer) {
  if (buffer == NULL || (buffer->size > 0 && buffer->stream == NULL))
    return NULL;

  if ((size_t)buffer->size == SIZE_MAX)
    return NULL;

  char *input = malloc((size_t)buffer->size + 1);

  if (input == NULL)
    return NULL;

  if (buffer->size > 0)
    memcpy(input, buffer->stream, buffer->size);

  input[buffer->size] = '\0';

  /* JSON input is text; embedded NUL bytes are invalid. */
  if (memchr(input, '\0', buffer->size) != NULL) {
    free(input);
    return NULL;
  }

  t_json_value *value = json_parse(input);

  free(input);
  return value;
}

/* Reader: public accessors */

t_json_type json_value_type(const t_json_value *value) {
  return value != NULL ? value->type : JSON_TYPE_NULL;
}

const char *json_value_string(const t_json_value *value) {
  if (value == NULL || value->type != JSON_TYPE_STRING)
    return NULL;

  return value->data.string;
}

int json_value_int(const t_json_value *value, int32_t *result) {
  if (value == NULL || result == NULL)
    return -1;

  if (value->type == JSON_TYPE_INT) {
    *result = value->data.int_value;
    return 0;
  }

  if (value->type == JSON_TYPE_UINT && value->data.uint_value <= INT32_MAX) {
    *result = (int32_t)value->data.uint_value;
    return 0;
  }

  return -1;
}

int json_value_uint(const t_json_value *value, uint32_t *result) {
  if (value == NULL || result == NULL)
    return -1;

  if (value->type == JSON_TYPE_UINT) {
    *result = value->data.uint_value;
    return 0;
  }

  if (value->type == JSON_TYPE_INT && value->data.int_value >= 0) {
    *result = (uint32_t)value->data.int_value;
    return 0;
  }

  return -1;
}

int json_value_double(const t_json_value *value, double *result) {
  if (value == NULL || result == NULL)
    return -1;

  switch (value->type) {
  case JSON_TYPE_DOUBLE:
    *result = value->data.double_value;
    return 0;
  case JSON_TYPE_INT:
    *result = value->data.int_value;
    return 0;
  case JSON_TYPE_UINT:
    *result = value->data.uint_value;
    return 0;
  default:
    return -1;
  }
}

int json_value_bool(const t_json_value *value, bool *result) {
  if (value == NULL || result == NULL || value->type != JSON_TYPE_BOOL)
    return -1;

  *result = value->data.bool_value;
  return 0;
}

const t_json_value *json_object_get(const t_json_value *object,
                                    const char *key) {
  if (object == NULL || key == NULL || object->type != JSON_TYPE_OBJECT)
    return NULL;

  for (size_t i = 0; i < object->data.object.size; i++) {
    if (strcmp(object->data.object.items[i].key, key) == 0)
      return object->data.object.items[i].value;
  }

  return NULL;
}

const t_json_value *json_array_get(const t_json_value *array, size_t index) {
  if (array == NULL || array->type != JSON_TYPE_ARRAY ||
      index >= array->data.array.size)
    return NULL;

  return array->data.array.items[index];
}

size_t json_array_size(const t_json_value *array) {
  if (array == NULL || array->type != JSON_TYPE_ARRAY)
    return 0;

  return array->data.array.size;
}

size_t json_object_size(const t_json_value *object) {
  if (object == NULL || object->type != JSON_TYPE_OBJECT)
    return 0;

  return object->data.object.size;
}
