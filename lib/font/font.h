#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <sys/types.h>

#define MAX_CHARACTER_SIZE 10

typedef const uint8_t FontData[96][MAX_CHARACTER_SIZE];

typedef struct Font {
  const uint8_t (*data)[MAX_CHARACTER_SIZE];
  size_t count;
  uint8_t width;
  uint8_t height;
} Font;

Font font_default();
Font font_eight_by_eight();
