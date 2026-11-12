#include "wallet.h"

extern OLED oled;

void wallet_display(int x, int y) {
  uint8_t w = ICON_WIDTH;
  uint8_t h = ICON_HEIGHT;
  Icon wallet_icon = icons_rpg_coin_gold_currency_money_gp();
  oled_draw_bitmap(&oled, x, y, w, h, false, wallet_icon);
  char wallet_label[256];
  snprintf(wallet_label, sizeof(wallet_label), "%" PRId32, wallet_get_coins());
  oled_write_text(&oled, x + w, y + h / 2, false, "%s", wallet_label);
}
