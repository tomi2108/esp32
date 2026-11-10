#include "api.h"
#include "inbox.h"

extern HttpClient *http_client;

void api_buy_item(int32_t item_id) {
  t_json *body = json_create();

  if (!body)
    return;

  json_object_start(body);
  json_key(body, "itemId");
  json_int(body, item_id);
  json_object_end(body);

  client_send_post(http_client, json_buffer(body), "/shop");
  json_destroy(body);
}

Response *api_publish_score(char *game_code, int32_t score) {
  t_json *body = json_create();
  if (!body)
    return NULL;

  json_object_start(body);

  json_key(body, "gameCode");
  json_string(body, game_code);

  json_key(body, "score");
  json_int(body, score);

  json_key(body, "playerId");
  json_int(body, PLAYER_ID);
  json_object_end(body);

  Response *res =
      client_send_post(http_client, json_buffer(body), "/minigames/score");
  json_destroy(body);
  return res;
}
