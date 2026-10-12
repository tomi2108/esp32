#include "menu.h"

void menu_next(Menu *menu) {

  if (menu->buzzer) {
    buzzer_set_frequency(menu->buzzer, 550);
    buzzer_tone_ms(*menu->buzzer, 100);
  }
  menu->current = (menu->current + 1) % menu->length;
  if (menu->on_next != NULL) {
    menu->on_next(menu->context);
  }
}

void menu_prev(Menu *menu) {
  if (menu->buzzer) {
    buzzer_set_frequency(menu->buzzer, 393);
    buzzer_tone_ms(*menu->buzzer, 100);
  }
  menu->current = (menu->current + menu->length - 1) % menu->length;
  if (menu->on_prev != NULL) {
    menu->on_prev(menu->context);
  }
}

void menu_select(Menu *menu) {
  if (menu->on_select != NULL) {
    menu->on_select(menu->context, menu->options[menu->current].value);
  }
  menu->selected = menu->current;
}
