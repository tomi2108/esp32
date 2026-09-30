#include "buffer.h"
#include <stdint.h>
#include <stdio.h>

t_buffer *buffer_create(void) {
  t_buffer *buffer = malloc(sizeof(t_buffer));
  if (buffer == NULL)
    return NULL;
  buffer->size = 0;
  buffer->offset = 0;
  buffer->stream = NULL;
  buffer->is_eof = false;
  return buffer;
}

void buffer_destroy(t_buffer *buffer) {
  free(buffer->stream);
  free(buffer);
}

int buffer_add(t_buffer *buffer, void *data, size_t size) {
  if (buffer->stream == NULL) {
    buffer->stream = malloc(size);
    if (buffer->stream == NULL)
      return -1;
  } else {
    buffer->stream = realloc(buffer->stream, buffer->size + size);
    if (buffer->stream == NULL)
      return -1;
  }
  memcpy(buffer->stream + buffer->size, data, size);
  buffer->size += size;
  return 0;
}

int buffer_read(t_buffer *buffer, void *data, size_t size) {
  if (buffer == NULL || data == NULL || size == 0)
    return -1;

  if (buffer->is_eof || buffer->offset + size > buffer->size) {
    buffer->is_eof = true;

    return -1;
  }

  memcpy(data, (uint8_t *)buffer->stream + buffer->offset, size);
  buffer->offset += size;
  return 0;
}

int buffer_add_uint8(t_buffer *buffer, uint8_t data) {
  return buffer_add(buffer, &data, sizeof(uint8_t));
}

int buffer_add_string(t_buffer *buffer, uint32_t length, char *string) {
  uint32_t l = sizeof(char) * (length + 1);
  buffer_add_uint32(buffer, l);
  return buffer_add(buffer, string, l);
}

uint32_t buffer_read_uint32(t_buffer *buffer) {
  uint32_t res = 0;
  if (buffer_read(buffer, &res, sizeof(uint32_t)) != 0)
    return 0;
  return res;
}

uint8_t buffer_read_uint8(t_buffer *buffer) {
  uint8_t res = 0;
  if (buffer_read(buffer, &res, sizeof(uint8_t)) != 0)
    return 0;
  return res;
}

char *buffer_read_string(t_buffer *buffer) {
  uint32_t length = buffer_read_uint32(buffer);
  char *res = malloc(length + 1);
  if (buffer_read(buffer, res, length) != 0)
    return NULL;
  res[length] = '\0';
  return res;
}

bool buffer_is_eof(t_buffer *buffer) {
  return buffer != NULL && buffer->is_eof;
}
