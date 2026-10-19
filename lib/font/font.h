
#ifndef FONT_H
#define FONT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint32_t codepoint;
  const uint8_t *bitmap;
  uint8_t width;
  uint8_t advance;
} FontGlyph;

typedef struct Font {
  const FontGlyph *glyphs;
  size_t count;
  uint8_t height;
  uint8_t line_height;
} Font;

Font font_default(void);
const FontGlyph *font_get_glyph(Font font, uint32_t codepoint);
uint32_t font_decode_next(const char **text);

#endif
