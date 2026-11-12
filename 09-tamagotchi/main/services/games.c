#include "games.h"

extern HttpClient *http_client;

int32_t game_publish_score(char *game_code, int32_t score) {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "gameCode");
  json_string(body, game_code);

  json_key(body, "score");
  json_int(body, score);

  json_key(body, "playerId");
  json_int(body, PLAYER_ID);
  json_object_end(body);

  headers_add(http_client->headers, "Content-Type", "application/json");
  Response *res =
      client_send_post(http_client, json_buffer(body), "/minigames/scores");
  json_destroy(body);
  headers_remove(http_client->headers, "Content-Type");

  t_json_value *res_body = json_parse_buffer(res->body);
  int32_t coins = 0;
  json_value_int(json_object_get(res_body, "coins"), &coins);
  return coins;
}
