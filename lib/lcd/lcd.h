#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"
#include <stdint.h>

#define LCD_ADDR 0x27
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_REG_SELECT 0x01

typedef struct LCD {
  i2c_master_dev_handle_t device;
  i2c_master_bus_handle_t bus;
  gpio_num_t sda;
  gpio_num_t scl;
  uint8_t rows;
  uint8_t columns;
} LCD;

void lcd_clear(LCD lcd);
void lcd_print_line(LCD lcd, uint8_t row, const char *text);
LCD lcd_init(uint8_t rows, uint8_t columns, gpio_num_t sda, gpio_num_t scl);
