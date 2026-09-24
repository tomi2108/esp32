#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "i2c.h"

#define OLED_ADDR 0x3C

typedef struct OLED {
  i2c_master_bus_handle_t bus;
  i2c_master_dev_handle_t device;
  uint8_t width;
  uint8_t height;
} OLED;

OLED oled_init(uint8_t width, uint8_t height, gpio_num_t sda, gpio_num_t scl);
void oled_write_text(OLED oled, uint8_t row, uint8_t column, const char *text);
