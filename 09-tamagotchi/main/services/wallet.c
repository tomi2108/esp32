#include "wallet.h"

extern HttpClient *http_client;

static int32_t wallet_coins = 0;

static void wallet_refresh_task(void *arg) {
  Response *wallet_res = client_send_get(http_client, "/wallet");
  if (wallet_res == NULL)
    return;

  t_json_value *wallet_body = json_parse_buffer(wallet_res->body);
  response_destroy(wallet_res);

  if (wallet_body == NULL)
    return;

  const t_json_value *coins = json_object_get(wallet_body, "coins");
  json_value_int(coins, &wallet_coins);

  json_value_destroy(wallet_body);
}

void wallet_refresh_async(void) {
  task_start("wallet_refresh", wallet_refresh_task, NULL);
}

int32_t wallet_get_coins(void) { return wallet_coins; }
