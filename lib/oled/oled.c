#include "oled.h"
#include <stdint.h>

static const uint8_t oled_init_sequence[] = {
    0xAE,       // Display OFF
    0xD5, 0x80, // Set display clock divide ratio / oscillator
    0xA8, 0x3F, // Multiplex ratio: 64
    0xD3, 0x00, // Display offset: 0
    0x40,       // Start line: 0
    0x8D, 0x14, // Charge pump ON
    0x20, 0x00, // Memory addressing mode: horizontal
    0xA1,       // Segment remap
    0xC8,       // COM output scan direction
    0xDA, 0x12, // COM pins hardware configuration
    0x81, 0x7F, // Contrast
    0xD9, 0xF1, // Pre-charge period
    0xDB, 0x40, // VCOMH deselect level
    0xA4,       // Entire display ON follows RAM
    0xA6,       // Normal display (not inverted)
    0xAF        // Display ON
};

static void oled_write_commands(OLED oled) {
  uint8_t buffer[17];
  int length = sizeof(oled_init_sequence);
  const uint8_t *commands = oled_init_sequence;

  while (length > 0) {
    size_t count = length;
    if (count > sizeof(buffer) - 1) {
      count = sizeof(buffer) - 1;
    }
    buffer[0] = 0x00;
    for (size_t i = 0; i < count; i++) {
      buffer[i + 1] = commands[i];
    }
    i2c_write(oled.device, buffer, count + 1);
    commands += count;
    length -= count;
  }
}

OLED oled_init(uint8_t width, uint8_t height, gpio_num_t sda, gpio_num_t scl) {
  i2c_master_bus_handle_t bus = i2c_bus_init(sda, scl);
  i2c_master_dev_handle_t device = i2c_device_init(bus, OLED_ADDR, 100000);
  OLED oled = {
      .bus = bus,
      .device = device,
      .width = width,
      .height = height,
  };
  oled_write_commands(oled);
  return oled;
}

void oled_write_text(OLED oled, uint8_t row, uint8_t column, const char *text) {

  return;
}
