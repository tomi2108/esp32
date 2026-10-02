#include <stdint.h>

typedef struct Option {
  char *key;
  int32_t value;
} Option;

typedef struct Menu {
  Option *options;
  int length;
  int selected;

  void *context;
  void (*on_next)(void *context);
  void (*on_prev)(void *context);
  void (*on_select)(void *context, int32_t value);
} Menu;

void menu_next(Menu *menu);
void menu_prev(Menu *menu);
void menu_select(Menu *menu);
