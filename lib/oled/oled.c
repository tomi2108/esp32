#include "oled.h"

static void oled_write_commands(OLED *oled, const uint8_t *commands,
                                size_t length) {
  uint8_t buffer[17];
  while (length > 0) {
    size_t count = length;

    if (count > sizeof(buffer) - 1)
      count = sizeof(buffer) - 1;

    buffer[0] = 0x00;

    for (size_t i = 0; i < count; i++)
      buffer[i + 1] = commands[i];

    i2c_write(oled->device, buffer, count + 1);
    commands += count;
    length -= count;
  }
}

void oled_init(OLED *oled, uint8_t width, uint8_t height, gpio_num_t sda,
               gpio_num_t scl) {
  oled->bus = i2c_bus_init(sda, scl);
  oled->device = i2c_device_init(oled->bus, OLED_ADDR, 100000);
  oled->width = width;
  oled->height = height;
  font_default(&oled->font);

  uint8_t multiplex = height - 1;
  uint8_t configuration[] = {
      0xAE, // Display OFF
      0xD5,
      0x80, // Set display clock divide ratio / oscillator frequency
      0xA8,
      multiplex, // Set multiplex ratio
      0xD3,
      0x00, // Set display offset to 0
      0x40, // Set display start line to 0
      0x8D,
      0x14, // Enable charge pump
      0x20,
      0x00, // Set horizontal addressing mode
      0xA1, // Segment remap
      0xC8, // Reverse COM output scan direction
      0xDA, // Set COM pins hardware configuration
      height == 32 ? 0x02 : 0x12,
      0x81,
      0x7F, // Set contrast
      0xD9,
      0xF1, // Set pre-charge period
      0xDB,
      0x40, // Set VCOMH deselect level
      0xA4, // Display follows RAM contents
      0xA6, // Normal display mode
      0x21,
      0x00, // Set column address range
      width - 1,
      0x22,
      0x00, // Set page address range
      (height + 7) / 8 - 1,
      0xAF, // Display ON
  };
  oled_write_commands(oled, configuration, sizeof(configuration));
  oled_clear(oled);
  oled_update(oled);
}

void oled_set_pixel(OLED *oled, uint8_t x, uint8_t y, bool on) {
  if (x >= oled->width || y >= oled->height)
    return;

  size_t index = x + (y / 8) * oled->width;
  uint8_t mask = 1 << (y % 8);

  if (on)
    oled->buffer[index] |= mask;
  else
    oled->buffer[index] &= ~mask;
}

void oled_write_char(OLED *oled, uint8_t x, uint8_t y, char character) {
  if (character < 0x20 || character > 0x7E)
    return;

  Font font = *oled->font;
  const uint8_t *glyph = font.data[character - 0x20];

  for (uint8_t column = 0; column < font.width; column++) {
    uint8_t bits = glyph[column];
    for (uint8_t row = 0; row < font.height; row++)
      oled_set_pixel(oled, x + column, y + row, bits & (1 << row));
  }
}
void oled_write_text(OLED *oled, uint8_t x, uint8_t y, const char *format,
                     ...) {
  char buffer[128];

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  const char *text = buffer;
  Font font = *oled->font;

  while (*text) {
    if (*text == '\n') {
      x = 0;
      y += font.height + 1;
      text++;
      continue;
    }

    if (x + font.width > oled->width) {
      x = 0;
      y += font.height + 1;
    }

    if (y + font.height >= oled->height)
      break;

    oled_write_char(oled, x, y, *text);
    x += font.width + 1;
    text++;
  }
}

void oled_update(OLED *oled) {
  uint8_t data[17];
  data[0] = 0x40;
  size_t size = oled->width * ((oled->height + 7) / 8);

  for (size_t offset = 0; offset < size;) {
    size_t count = size - offset;
    if (count > sizeof(data) - 1)
      count = sizeof(data) - 1;
    for (size_t i = 0; i < count; i++)
      data[i + 1] = oled->buffer[offset + i];
    i2c_write(oled->device, data, count + 1);
    offset += count;
  }
}

void oled_clear(OLED *oled) {
  size_t size = oled->width * ((oled->height + 7) / 8);
  memset(oled->buffer, 0, size);
}
