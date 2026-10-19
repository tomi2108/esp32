#ifndef OLED_H
#define OLED_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"
#include "font.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"
#include "menu.h"
#include "rom/ets_sys.h"
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PI 3.14159265f
#define OLED_ADDR 0x3C
#define OLED_MAX_WIDTH 128
#define OLED_MAX_HEIGHT 128
#define OLED_BUFFER_SIZE ((OLED_MAX_WIDTH * OLED_MAX_HEIGHT) / 8)
typedef enum {
  OLED_CONTROLLER_SSD1306,
  OLED_CONTROLLER_SH1106,
} OLEDController;

typedef struct OLED {
  i2c_master_bus_handle_t bus;
  i2c_master_dev_handle_t device;
  uint8_t width;
  uint8_t height;
  uint8_t buffer[OLED_BUFFER_SIZE];
  Font font;
  OLEDController controller;
} OLED;

void oled_clear(OLED *oled);
void oled_update(OLED *oled);

void oled_init(OLED *oled, uint8_t width, uint8_t height, gpio_num_t sda,
               gpio_num_t scl, OLEDController controller);

void oled_write_text(OLED *oled, uint8_t x, uint8_t y, bool inverted,
                     const char *format, ...);

void oled_write_char(OLED *oled, uint8_t x, uint8_t y, uint32_t codepoint);
void oled_write_char_inverted(OLED *oled, uint8_t x, uint8_t y,
                              uint32_t codepoint);

void oled_set_pixel(OLED *oled, uint8_t x, uint8_t y, bool on);

void oled_display_menu(OLED *oled, Menu menu);

void oled_draw_spinner(OLED *oled, int cx, int cy, int radius);
void oled_draw_bitmap(OLED *oled, uint8_t x, uint8_t y, uint8_t width,
                      uint8_t height, bool inverted, const uint8_t *bitmap);

void oled_draw_dot(OLED *oled, int cx, int cy, int radius, bool on);
void oled_display_menu_with_dots(OLED *oled, Menu menu, const bool *dots);
void oled_draw_progress_bar(OLED *oled, uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, double value, bool vertical);
#endif
