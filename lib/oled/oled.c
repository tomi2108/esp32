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

static uint8_t oled_draw_menu_header(OLED *oled, const OLEDMenuHeader *header) {
  if (!header || !header->title || header->title[0] == '\0')
    return 0;

  uint8_t height = oled->font.line_height + 4;

  if (height >= oled->height)
    height = oled->height / 2;

  if (header->inverted) {
    for (uint8_t y = 0; y < height; y++) {
      for (uint8_t x = 0; x < oled->width; x++)
        oled_set_pixel(oled, x, y, true);
    }
  }

  oled_write_text(oled, MENU_PADDING_X, 2, header->inverted, "%s",
                  header->title);

  if (header->separator) {
    for (uint8_t x = 0; x < oled->width; x++)
      oled_set_pixel(oled, x, height - 1, !header->inverted);
  }

  return height;
}

static uint16_t oled_text_width(OLED *oled, const char *text) {
  uint16_t width = 0;

  while (*text) {
    const char *next = text;
    uint32_t codepoint = font_decode_next(&next);
    const FontGlyph *glyph = font_get_glyph(oled->font, codepoint);

    if (!glyph)
      break;

    width += glyph->advance;
    text = next;
  }

  return width;
}

static void oled_display_menu_item(OLED *oled, const OLEDMenu *config,
                                   size_t index, uint8_t y,
                                   uint8_t row_height) {
  Menu menu = config->menu;
  const char *key = menu.options[index].key;

  if (!key)
    return;

  const OLEDMenuItemDecoration *decoration =
      config->decorations ? &config->decorations[index] : NULL;

  const uint8_t *bitmap = decoration ? decoration->bitmap : NULL;
  const char *right_label =
      decoration && decoration->label ? decoration->label : "";

  uint8_t bitmap_width = config->bitmap_width;
  uint8_t bitmap_height = config->bitmap_height;

  bool has_bitmap = bitmap && bitmap_width > 0 && bitmap_height > 0 &&
                    bitmap_height <= row_height;

  uint16_t right_label_width = oled_text_width(oled, right_label);
  uint16_t right_width = right_label_width;

  if (has_bitmap)
    right_width += bitmap_width + 2;

  uint16_t text_x = config->padding_x;
  uint16_t text_max_width = oled->width - text_x - 2;

  if (right_width > 0) {
    uint16_t right_start =
        right_width + 2 < oled->width ? oled->width - right_width - 2 : 0;

    text_max_width = right_start > text_x + 2 ? right_start - text_x - 2 : 0;
  }

  uint16_t text_width = 0;
  const char *cursor = key;

  while (*cursor) {
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

  size_t text_length = cursor - key;
  char text[text_length + 1];

  memcpy(text, key, text_length);
  text[text_length] = '\0';

  uint8_t text_y = y + (row_height - oled->font.height) / 2;

  oled_write_text(oled, text_x, text_y, false, "%s", text);

  uint16_t right_x = oled->width - right_width - 2;

  if (has_bitmap) {
    uint8_t bitmap_y = y + (row_height - bitmap_height) / 2;

    oled_draw_bitmap(oled, right_x, bitmap_y, bitmap_width, bitmap_height,
                     false, bitmap);
    right_x += bitmap_width + 2;
  }

  if (right_label_width > 0)
    oled_write_text(oled, right_x, text_y, false, "%s", right_label);

  bool selected = index == menu.current;
  uint16_t border_right =
      right_width > 0 ? oled->width - 1 : text_x + text_width + 1;

  if (border_right >= oled->width)
    border_right = oled->width - 1;

  for (uint8_t row = 0; row < row_height; row++) {
    oled_set_pixel(oled, 0, y + row, selected);
    oled_set_pixel(oled, border_right, y + row, selected);
  }

  for (uint16_t x = 0; x <= border_right; x++) {
    oled_set_pixel(oled, x, y, selected);
    oled_set_pixel(oled, x, y + row_height - 1, selected);
  }
}

void oled_display_menu(OLED *oled, const OLEDMenu *config) {
  if (!oled || !config)
    return;

  oled_clear(oled);

  if (oled->font.line_height == 0)
    return;

  uint8_t row_height =
      config->row_height ? config->row_height : MENU_ROW_HEIGHT;
  uint8_t padding_x = config->padding_x ? config->padding_x : MENU_PADDING_X;

  if (row_height < oled->font.height || row_height == 0)
    return;

  OLEDMenu normalized = *config;
  normalized.row_height = row_height;
  normalized.padding_x = padding_x;

  uint8_t header_height = oled_draw_menu_header(oled, config->header);
  uint8_t available_height = oled->height - header_height;
  uint8_t visible = available_height / row_height;

  if (visible == 0)
    return;

  Menu menu = config->menu;
  size_t scroll_offset = 0;

  if (menu.length > visible) {
    if (menu.current >= visible / 2)
      scroll_offset = menu.current - visible / 2;

    if (scroll_offset + visible > menu.length)
      scroll_offset = menu.length - visible;
  }

  for (size_t i = 0; i < visible; i++) {
    size_t index = scroll_offset + i;

    if (index >= menu.length)
      break;

    uint16_t y = header_height + i * row_height;

    if (y + row_height > oled->height)
      break;

    oled_display_menu_item(oled, &normalized, index, y, row_height);
  }
}

void oled_draw_line(OLED *oled, int x0, int y0, int x1, int y1, bool on) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);

  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;

  int error = dx - dy;

  while (true) {
    oled_set_pixel(oled, x0, y0, on);

    if (x0 == x1 && y0 == y1)
      break;

    int error2 = 2 * error;

    if (error2 > -dy) {
      error -= dy;
      x0 += sx;
    }

    if (error2 < dx) {
      error += dx;
      y0 += sy;
    }
  }
}

void oled_draw_circle(OLED *oled, int cx, int cy, int inner_radius,
                      int outer_radius, bool on) {
  if (!oled || inner_radius < 0 || outer_radius < inner_radius)
    return;

  int inner_sq = inner_radius * inner_radius;
  int outer_sq = outer_radius * outer_radius;

  for (int dy = -outer_radius; dy <= outer_radius; dy++) {
    for (int dx = -outer_radius; dx <= outer_radius; dx++) {
      int distance_sq = dx * dx + dy * dy;

      if (distance_sq >= inner_sq && distance_sq <= outer_sq)
        oled_set_pixel(oled, cx + dx, cy + dy, on);
    }
  }
}

void oled_draw_rectangle(OLED *oled, int x, int y, int width, int height,
                         bool on) {
  if (!oled || width <= 0 || height <= 0)
    return;

  oled_draw_line(oled, x, y, x + width - 1, y, on);
  oled_draw_line(oled, x, y, x, y + height - 1, on);
  oled_draw_line(oled, x + width - 1, y, x + width - 1, y + height - 1, on);
  oled_draw_line(oled, x, y + height - 1, x + width - 1, y + height - 1, on);
}

void oled_fill_rectangle(OLED *oled, int x, int y, int width, int height,
                         bool on) {
  if (!oled || width <= 0 || height <= 0)
    return;

  for (int dy = 0; dy < height; dy++) {
    for (int dx = 0; dx < width; dx++)
      oled_set_pixel(oled, x + dx, y + dy, on);
  }
}

void oled_draw_square(OLED *oled, int x, int y, int size, bool on) {
  oled_draw_rectangle(oled, x, y, size, size, on);
}

void oled_fill_square(OLED *oled, int x, int y, int size, bool on) {
  oled_fill_rectangle(oled, x, y, size, size, on);
}

void oled_draw_modal(OLED *oled, Modal *modal) {
  const int padding = 2;
  const int line_height = oled->font.height + 1;
  const int content_x = modal->x + padding + 2;
  const int content_y = modal->y + padding + 2;
  oled_fill_rectangle(oled, modal->x, modal->y, modal->width, modal->height,
                      true);
  oled_fill_rectangle(oled, modal->x + padding, modal->y + padding,
                      modal->width - padding * 2, modal->height - padding * 2,
                      false);
  if (modal->title != NULL)
    oled_write_text(oled, content_x, content_y, false, "%s", modal->title);
  if (modal->description != NULL)
    oled_write_text(oled, content_x, content_y + line_height, false, "%s",
                    modal->description);
  int item_y = content_y + line_height * 2;

  for (size_t i = 0; i < modal->item_count; i++) {
    const ModalItem *item = &modal->items[i];
    int item_x = content_x;
    int text_y = item_y;

    if (item->bitmap != NULL) {
      oled_draw_bitmap(oled, item_x, item_y, item->bitmap_width,
                       item->bitmap_height, false, item->bitmap);

      item_x += item->bitmap_width + 2;
      text_y = item_y + item->bitmap_height / 2;
    }

    if (item->label != NULL)
      oled_write_text(oled, item_x, text_y, false, "%s: %d", item->label,
                      item->value);
    else
      oled_write_text(oled, item_x, text_y, false, "%d", item->value);

    item_y += item->bitmap != NULL && item->bitmap_height > line_height
                  ? item->bitmap_height
                  : line_height;
  }
  int options_y =
      modal->y + modal->height - padding - line_height * modal->menu->length;
  for (size_t i = 0; i < modal->menu->length; i++)
    oled_write_text(oled, content_x, options_y + line_height * i,
                    modal->menu->current == i, "%s",
                    modal->menu->options[i].key);
}
