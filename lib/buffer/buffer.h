#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint32_t size;
  uint32_t offset;
  void *stream;
} t_buffer;

t_buffer *buffer_create(void);
void buffer_destroy(t_buffer *buffer);

int buffer_add(t_buffer *buffer, void *data, size_t size);
void buffer_read(t_buffer *buffer, void *data, size_t size);

int buffer_add_uint8(t_buffer *buffer, uint8_t data);
int buffer_add_uint32(t_buffer *buffer, uint32_t data);
int buffer_add_string(t_buffer *buffer, uint32_t length, char *string);

uint8_t buffer_read_uint8(t_buffer *buffer);
uint32_t buffer_read_uint32(t_buffer *buffer);
char *buffer_read_string(t_buffer *buffer);
