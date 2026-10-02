#include "menu.h"

void menu_next(Menu *menu) {
  menu->selected = (menu->selected + 1) % menu->length;
  menu->on_next(menu->context);
}

void menu_prev(Menu *menu) {
  menu->selected = (menu->selected + menu->length - 1) % menu->length;
  menu->on_prev(menu->context);
}

void menu_select(Menu *menu) {
  menu->on_select(menu->context, menu->options[menu->selected].value);
}
