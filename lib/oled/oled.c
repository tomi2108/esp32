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
               gpio_num_t scl, OLEDController controller) {
  oled->bus = i2c_bus_init(sda, scl);
  oled->device = i2c_device_init(oled->bus, OLED_ADDR, 100000);
  oled->width = width;
  oled->height = height;
  oled->font = font_default();
  oled->controller = controller;

  uint8_t multiplex = height - 1;

  uint8_t common[] = {
      0xAE, 0xD5, 0x80, 0xA8, multiplex, 0xD3, 0x00,
      0x40, 0x8D, 0x14, 0xA1, 0xC8,      0xDA, height == 32 ? 0x02 : 0x12,
      0x81, 0x7F, 0xD9, 0xF1, 0xDB,      0x40, 0xA4,
      0xA6,
  };

  oled_write_commands(oled, common, sizeof(common));

  if (controller == OLED_CONTROLLER_SSD1306) {
    uint8_t addressing[] = {
        0x20,
        0x00,
        0x21,
        0x00,
        (uint8_t)(width - 1),
        0x22,
        0x00,
        (uint8_t)((height + 7) / 8 - 1),
    };

    oled_write_commands(oled, addressing, sizeof(addressing));
  }

  if (controller == OLED_CONTROLLER_SH1106) {
    uint8_t addressing[] = {
        0xAD,
        0x8B, // Enable DC-DC converter
        0xA8,
        multiplex,
    };

    oled_write_commands(oled, addressing, sizeof(addressing));
  }

  uint8_t display_on = 0xAF;
  oled_write_commands(oled, &display_on, 1);

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

void oled_write_char_mode(OLED *oled, uint8_t x, uint8_t y, char character,
                          bool inverted) {
  if (character < 0x20 || character > 0x7E)
    return;

  Font font = oled->font;
  const uint8_t *glyph = font.data[character - 0x20];

  for (uint8_t column = 0; column < font.width; column++) {
    uint8_t bits = glyph[column];
    for (uint8_t row = 0; row < font.height; row++) {
      bool on = bits & (1 << row);
      if (inverted)
        on = !on;
      oled_set_pixel(oled, x + column, y + row, on);
    }
  }
}

void oled_write_char(OLED *oled, uint8_t x, uint8_t y, char character) {
  oled_write_char_mode(oled, x, y, character, false);
}
void oled_write_char_inverted(OLED *oled, uint8_t x, uint8_t y,
                              char character) {
  oled_write_char_mode(oled, x, y, character, true);
}

void oled_write_text(OLED *oled, uint8_t x, uint8_t y, bool inverted,
                     const char *format, ...) {
  char buffer[128];

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  const char *text = buffer;
  Font font = oled->font;

  while (*text) {
    if (*text == '\n') {
      x = 0;
      y += font.height + 1;
      text++;
      continue;
    }

    if (x + font.width > oled->width) {
      x = 0;
      y += font.height;
    }

    if (y + font.height > oled->height)
      break;

    oled_write_char_mode(oled, x, y, *text, inverted);
    x += font.width;
    text++;
  }
}
void oled_update(OLED *oled) {
  uint8_t data[17];
  size_t pages = (oled->height + 7) / 8;

  if (oled->controller == OLED_CONTROLLER_SSD1306) {
    uint8_t addressing[] = {
        0x21, 0x00, (uint8_t)(oled->width - 1),
        0x22, 0x00, (uint8_t)(pages - 1),
    };

    oled_write_commands(oled, addressing, sizeof(addressing));

    data[0] = 0x40;
    size_t size = oled->width * pages;

    for (size_t offset = 0; offset < size;) {
      size_t count = size - offset;

      if (count > sizeof(data) - 1)
        count = sizeof(data) - 1;

      memcpy(data + 1, oled->buffer + offset, count);
      i2c_write(oled->device, data, count + 1);
      offset += count;
    }

    return;
  }

  if (oled->controller == OLED_CONTROLLER_SH1106) {
    for (size_t page = 0; page < pages; page++) {
      uint8_t commands[] = {
          (uint8_t)(0xB0 | page),
          0x02, // Lower column address; offset 2
          0x10, // Upper column address
      };

      oled_write_commands(oled, commands, sizeof(commands));

      data[0] = 0x40;

      for (size_t offset = 0; offset < oled->width;) {
        size_t count = oled->width - offset;

        if (count > sizeof(data) - 1)
          count = sizeof(data) - 1;

        memcpy(data + 1, oled->buffer + page * oled->width + offset, count);

        i2c_write(oled->device, data, count + 1);
        offset += count;
      }
    }
  }
}

void oled_clear(OLED *oled) {
  size_t size = oled->width * ((oled->height + 7) / 8);
  memset(oled->buffer, 0, size);
}

void oled_display_menu(OLED *oled, Menu menu) {
  oled_clear(oled);
  uint8_t line_height = oled->font.height + 1;
  uint8_t visible = oled->height / line_height;
  uint8_t scroll_offset = 0;
  size_t max_chars = oled->width / oled->font.width;

  if (menu.length > visible) {
    if (menu.current >= visible / 2)
      scroll_offset = menu.current - visible / 2;
    if (scroll_offset + visible > menu.length)
      scroll_offset = menu.length - visible;
  }

  for (uint8_t i = 0; i < visible; i++) {
    uint8_t index = scroll_offset + i;

    if (index >= menu.length)
      break;

    const char *key = menu.options[index].key;
    if (key == NULL)
      continue;

    uint8_t y = i * line_height;
    bool is_selected = index == menu.current;
    size_t len = strlen(key);
    char text[max_chars + 1];

    if (len > max_chars) {
      if (max_chars <= 3) {
        memset(text, '.', max_chars);
        text[max_chars] = '\0';
      } else {
        memcpy(text, key, max_chars - 3);
        memcpy(text + max_chars - 3, "...", 3);
        text[max_chars] = '\0';
      }
    } else {
      memcpy(text, key, len + 1);
    }

    size_t width = strlen(text) * oled->font.width;

    oled_write_text(oled, 1, y + 1, is_selected, "%s", text);

    for (uint8_t row = 0; row < oled->font.height + 2; row++) {
      oled_set_pixel(oled, 0, y + row, is_selected);
      oled_set_pixel(oled, width + 1, y + row, is_selected);
    }

    for (uint8_t x = 0; x <= width + 1; x++) {
      oled_set_pixel(oled, x, y, is_selected);
      oled_set_pixel(oled, x, y + oled->font.height + 1, is_selected);
    }
  }
}

static void draw_dot(OLED *oled, int cx, int cy, int radius, bool on) {
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      if (dx * dx + dy * dy <= radius * radius) {
        oled_set_pixel(oled, cx + dx, cy + dy, on);
      }
    }
  }
}

void oled_draw_spinner(OLED *oled, int cx, int cy, int radius) {
  int frame = (esp_timer_get_time() / 100000) % 8;

  for (int i = 0; i < 8; i++) {
    float angle = 2.0f * PI * i / 8;

    int x = cx + (int)(cosf(angle) * radius);
    int y = cy + (int)(sinf(angle) * radius);

    int distance = (i - frame + 8) % 8;
    bool on = distance < 4;

    draw_dot(oled, x, y, 2, on);
  }
}

void oled_draw_bitmap(OLED *oled, uint8_t x, uint8_t y, uint8_t width,
                      uint8_t height, const uint8_t *bitmap) {
  if (!oled || !bitmap || width == 0 || height == 0)
    return;

  size_t bytes_per_row = (width + 7) / 8;

  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      size_t index = row * bytes_per_row + col / 8;
      uint8_t mask = 1U << (7 - col % 8);
      bool on = (bitmap[index] & mask) != 0;

      oled_set_pixel(oled, x + col, y + row, on);
    }
  }
}

void oled_draw_progress_bar(OLED *oled, uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, double value) {
  if (value < 0)
    value = 0;
  if (value > 100)
    value = 100;

  for (uint8_t dx = 0; dx < width; dx++) {
    oled_set_pixel(oled, x + dx, y, true);
    oled_set_pixel(oled, x + dx, y + height - 1, true);
  }

  for (uint8_t dy = 0; dy < height; dy++) {
    oled_set_pixel(oled, x, y + dy, true);
    oled_set_pixel(oled, x + width - 1, y + dy, true);
  }

  uint8_t fill_width = (uint8_t)((width - 2) * value / 100.0);

  for (uint8_t dy = 1; dy < height - 1; dy++) {
    for (uint8_t dx = 1; dx <= fill_width; dx++) {
      oled_set_pixel(oled, x + dx, y + dy, true);
    }
  }
}
