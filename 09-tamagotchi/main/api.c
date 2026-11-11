#include "api.h"
#include "inbox.h"
#include <stdint.h>

extern HttpClient *http_client;

void api_buy_item(int32_t item_id) {
  t_json *body = json_create();
  json_object_start(body);
  json_key(body, "itemId");
  json_int(body, item_id);
  json_object_end(body);

  client_send_post(http_client, json_buffer(body), "/shop");
  json_destroy(body);
}

int32_t api_publish_score(char *game_code, int32_t score) {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "gameCode");
  json_string(body, game_code);

  json_key(body, "score");
  json_int(body, score);

  json_key(body, "playerId");
  json_int(body, PLAYER_ID);
  json_object_end(body);

  Response *res =
      client_send_post(http_client, json_buffer(body), "/minigames/scores");
  json_destroy(body);

  t_json_value *res_body = json_parse_buffer(res->body);
  int32_t coins;
  json_value_int(json_object_get(res_body, "coins"), &coins);
  return coins;
}
