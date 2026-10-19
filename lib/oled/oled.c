
#include "oled.h"

#define MENU_ROW_HEIGHT 16
#define MENU_PADDING_X 1

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
        0x8B,
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
  uint8_t mask = 1U << (y % 8);

  if (on)
    oled->buffer[index] |= mask;
  else
    oled->buffer[index] &= (uint8_t)~mask;
}

void oled_write_char_mode(OLED *oled, uint8_t x, uint8_t y, uint32_t codepoint,
                          bool inverted) {
  const FontGlyph *glyph = font_get_glyph(oled->font, codepoint);

  if (!glyph)
    return;

  uint8_t width = glyph->width;

  if (inverted && glyph->advance > width)
    width = glyph->advance;

  for (uint8_t column = 0; column < width; column++) {
    for (uint8_t row = 0; row < oled->font.height; row++) {
      bool on = false;

      if (column < glyph->width)
        on = (glyph->bitmap[column] & (1U << row)) != 0;

      if (inverted)
        on = !on;

      oled_set_pixel(oled, x + column, y + row, on);
    }
  }
}

void oled_write_char(OLED *oled, uint8_t x, uint8_t y, uint32_t codepoint) {
  oled_write_char_mode(oled, x, y, codepoint, false);
}

void oled_write_char_inverted(OLED *oled, uint8_t x, uint8_t y,
                              uint32_t codepoint) {
  oled_write_char_mode(oled, x, y, codepoint, true);
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
  uint8_t start_x = x;

  while (*text) {
    if (*text == '\n') {
      x = start_x;
      y += font.line_height;
      text++;
      continue;
    }

    uint32_t codepoint = font_decode_next(&text);
    const FontGlyph *glyph = font_get_glyph(font, codepoint);

    if (!glyph)
      continue;

    if (x + glyph->width > oled->width) {
      x = start_x;
      y += font.line_height;
    }

    if (y + font.height > oled->height)
      break;

    oled_write_char_mode(oled, x, y, codepoint, inverted);
    x += glyph->advance;
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
          0x02,
          0x10,
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

void oled_draw_dot(OLED *oled, int cx, int cy, int radius, bool on) {
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      if (dx * dx + dy * dy <= radius * radius)
        oled_set_pixel(oled, cx + dx, cy + dy, on);
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
    oled_draw_dot(oled, x, y, 2, distance < 4);
  }
}

void oled_draw_bitmap(OLED *oled, uint8_t x, uint8_t y, uint8_t width,
                      uint8_t height, bool inverted, const uint8_t *bitmap) {
  if (!oled || !bitmap || width == 0 || height == 0)
    return;

  size_t bytes_per_row = (width + 7) / 8;

  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      size_t index = row * bytes_per_row + col / 8;
      uint8_t mask = 1U << (7 - col % 8);
      bool on = (bitmap[index] & mask) != 0;
      oled_set_pixel(oled, x + col, y + row, inverted ? !on : on);
    }
  }
}

void oled_draw_progress_bar(OLED *oled, uint8_t x, uint8_t y, uint8_t width,
                            uint8_t height, double value, bool vertical) {
  if (width < 2 || height < 2)
    return;

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

  if (vertical) {
    uint8_t fill_height = (uint8_t)((height - 2) * value / 100.0);

    for (uint8_t dy = 1; dy <= fill_height; dy++) {
      for (uint8_t dx = 1; dx < width - 1; dx++)
        oled_set_pixel(oled, x + dx, y + height - 1 - dy, true);
    }
  } else {
    uint8_t fill_width = (uint8_t)((width - 2) * value / 100.0);

    for (uint8_t dy = 1; dy < height - 1; dy++) {
      for (uint8_t dx = 1; dx <= fill_width; dx++)
        oled_set_pixel(oled, x + dx, y + dy, true);
    }
  }
}
static void oled_display_menu_internal(OLED *oled, Menu menu,
                                       const uint8_t *const *bitmaps,
                                       const char *const *right_labels,
                                       uint8_t bitmap_width,
                                       uint8_t bitmap_height) {
  oled_clear(oled);

  if (oled->font.line_height == 0)
    return;

  uint8_t visible = oled->height / MENU_ROW_HEIGHT;
  uint8_t scroll_offset = 0;

  if (visible == 0)
    return;

  if (menu.length > visible) {
    if (menu.current >= visible / 2)
      scroll_offset = menu.current - visible / 2;

    if (scroll_offset + visible > menu.length)
      scroll_offset = menu.length - visible;
  }

  for (uint8_t i = 0; i < visible; i++) {
    size_t index = scroll_offset + i;

    if (index >= menu.length)
      break;

    const char *key = menu.options[index].key;

    if (!key)
      continue;

    uint8_t y = i * MENU_ROW_HEIGHT;
    bool is_selected = index == menu.current;

    bool has_bitmap = bitmaps && bitmaps[index] && bitmap_width > 0 &&
                      bitmap_height > 0 && bitmap_height <= MENU_ROW_HEIGHT;

    const char *right_label =
        right_labels && right_labels[index] ? right_labels[index] : "";

    uint16_t right_label_width = 0;
    const char *cursor = right_label;

    while (*cursor != '\0') {
      const char *next = cursor;
      uint32_t codepoint = font_decode_next(&next);
      const FontGlyph *glyph = font_get_glyph(oled->font, codepoint);

      if (!glyph)
        break;

      right_label_width += glyph->advance;
      cursor = next;
    }

    uint16_t right_width = right_label_width;

    if (has_bitmap)
      right_width += bitmap_width + 2;

    uint16_t text_x = MENU_PADDING_X;
    uint16_t text_max_width = oled->width - MENU_PADDING_X - 2;

    if (right_width > 0) {
      uint16_t right_start = oled->width - right_width - 2;

      if (right_start <= text_x)
        text_max_width = 0;
      else
        text_max_width = right_start - text_x - 2;
    }

    uint16_t text_width = 0;
    cursor = key;

    while (*cursor != '\0') {
      const char *next = cursor;
      uint32_t codepoint = font_decode_next(&next);
      const FontGlyph *glyph = font_get_glyph(oled->font, codepoint);

      if (!glyph)
        break;

      if (text_width + glyph->advance > text_max_width)
        break;

      text_width += glyph->advance;
      cursor = next;
    }

    char text[strlen(key) + 1];
    size_t text_len = cursor - key;

    memcpy(text, key, text_len);
    text[text_len] = '\0';

    uint8_t text_y = y + (MENU_ROW_HEIGHT - oled->font.height) / 2;

    oled_write_text(oled, text_x, text_y, false, "%s", text);

    uint16_t right_x = oled->width - right_width - 2;

    if (has_bitmap) {
      uint8_t bitmap_y = y + (MENU_ROW_HEIGHT - bitmap_height) / 2;
      oled_draw_bitmap(oled, right_x, bitmap_y, bitmap_width, bitmap_height,
                       false, bitmaps[index]);
      right_x += bitmap_width + 2;
    }

    if (right_label_width > 0)
      oled_write_text(oled, right_x, text_y, false, "%s", right_label);

    uint16_t border_right =
        right_width > 0 ? oled->width - 1 : text_x + text_width + 1;

    if (border_right >= oled->width)
      border_right = oled->width - 1;

    for (uint8_t row = 0; row < MENU_ROW_HEIGHT; row++) {
      oled_set_pixel(oled, 0, y + row, is_selected);
      oled_set_pixel(oled, border_right, y + row, is_selected);
    }

    for (uint16_t x = 0; x <= border_right; x++) {
      oled_set_pixel(oled, x, y, is_selected);
      oled_set_pixel(oled, x, y + MENU_ROW_HEIGHT - 1, is_selected);
    }
  }
}

void oled_display_menu(OLED *oled, Menu menu) {
  oled_display_menu_internal(oled, menu, NULL, NULL, 0, 0);
}

void oled_display_menu_with_bitmaps(OLED *oled, Menu menu,
                                    const uint8_t *const *bitmaps,
                                    const char *const *right_labels,
                                    uint8_t bitmap_width,
                                    uint8_t bitmap_height) {
  oled_display_menu_internal(oled, menu, bitmaps, right_labels, bitmap_width,
                             bitmap_height);
}
