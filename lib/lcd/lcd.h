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

void lcd_clear(i2c_master_dev_handle_t lcd_device);
void lcd_print(i2c_master_dev_handle_t lcd_device, const char *text);
void lcd_set_cursor(i2c_master_dev_handle_t lcd_device, uint8_t row,
                    uint8_t column);
void lcd_init(i2c_master_bus_handle_t *i2c_bus,
              i2c_master_dev_handle_t *lcd_device, gpio_num_t sda,
              gpio_num_t scl);
