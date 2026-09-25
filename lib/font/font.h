#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <sys/types.h>

typedef const uint8_t Font[96][5];
uint32_t font_default(Font **out);
