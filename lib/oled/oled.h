#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "font.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"
#include "rom/ets_sys.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define OLED_ADDR 0x3C
#define OLED_MAX_WIDTH 128
#define OLED_MAX_HEIGHT 128
#define OLED_BUFFER_SIZE ((OLED_MAX_WIDTH * OLED_MAX_HEIGHT) / 8)

typedef struct OLED {
  i2c_master_bus_handle_t bus;
  i2c_master_dev_handle_t device;
  uint8_t width;
  uint8_t height;
  uint8_t buffer[OLED_BUFFER_SIZE];
  Font *font;
} OLED;

void oled_init(OLED *oled, uint8_t width, uint8_t height, gpio_num_t sda,
               gpio_num_t scl);
void oled_write_text(OLED *oled, uint8_t x, uint8_t y, const char *format, ...);
void oled_write_char(OLED *oled, uint8_t x, uint8_t y, char character);
void oled_set_pixel(OLED *oled, uint8_t x, uint8_t y, bool on);
void oled_clear(OLED *oled);
void oled_update(OLED *oled);
