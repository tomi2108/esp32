#ifndef MODAL_H
#define MODAL_H

#include "menu.h"

typedef struct {
  const uint8_t *bitmap;
  const char *label;
  int bitmap_width;
  int bitmap_height;
  int value;
} ModalItem;

typedef struct {
  const char *title;
  const char *description;
  ModalItem *items;
  size_t item_count;
  Menu *menu;
  int x;
  int y;
  int width;
  int height;
} Modal;

#endif
