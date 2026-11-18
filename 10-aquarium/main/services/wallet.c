#include "wallet.h"

static int32_t wallet_coins = 0;

int32_t wallet_get_coins(void) { return wallet_coins; };

int wallet_add_coins(int32_t coins) {
  wallet_coins += coins;
  return 1;
};

int wallet_remove_coins(int32_t coins) {
  if (wallet_coins < coins)
    return 0;
  wallet_coins -= coins;
  return 1;
};
