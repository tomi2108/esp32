#include "missions.h"

extern HttpClient *http_client;

static Mission automatic[MAX_MISSIONS_SIZE];
static int automatic_size = 0;

static Mission manual[MAX_MISSIONS_SIZE];
static int manual_size = 0;

const char *next_reset_at;

Mission *missions_get(MissionType type) {
  return type == MISSIONS_AUTOMATIC ? automatic : manual;
}

int missions_get_size(MissionType type) {
  return type == MISSIONS_AUTOMATIC ? automatic_size : manual_size;
}

static void decode_mission_item(const t_json_value *body, Mission *out) {}

static void missions_refresh_task(void *arg) {
  Response *res = client_send_get(http_client, "/inventory");
  if (res == NULL || res->body == NULL)
    return;

  t_json_value *body = json_parse_buffer(res->body);
  if (body == NULL)
    return;

  const t_json_value *missions = json_object_get(body, "missions");
  next_reset_at = json_value_string(json_object_get(body, "nextResetAt"));

  automatic_size = 0;
  manual_size = 0;

  int count = json_array_size(missions);
  for (int i = 0; i < count; i++) {
    Mission item = {0};
    decode_mission_item(json_array_get(missions, i), &item);

    if (strcmp(item.type, "automatic") == 0) {
      if (automatic_size < MAX_MISSIONS_SIZE)
        automatic[automatic_size++] = item;
    } else if (strcmp(item.type, "manual") == 0) {
      if (manual_size < MAX_MISSIONS_SIZE)
        manual[manual_size++] = item;
    }
  }
  return;
}

void missions_refresh_async(void) {
  task_start("mission_refresh", missions_refresh_task, NULL);
}

static int missions_find(MissionType type, int32_t mission_id) {
  Mission *missions = missions_get(type);
  int size = missions_get_size(type);

  for (int i = 0; i < size; i++) {
    if (missions[i].id == mission_id)
      return i;
  }

  return -1;
}

static void missions_remove(MissionType type, int index) {
  Mission *missions = missions_get(type);
  int size = missions_get_size(type);
  if (index < 0 || index >= size)
    return;

  for (int i = index; i < size - 1; i++)
    missions[i] = missions[i + 1];
  if (type == MISSIONS_AUTOMATIC)
    automatic_size--;
  if (type == MISSIONS_MANUAL)
    manual_size--;
}

void mission_complete_task(void *args) {
  int32_t mission_id = *(int32_t *)args;
  t_json *body = json_create();
  json_object_start(body);
  json_key(body, "missionId");
  json_int(body, mission_id);
  json_object_end(body);

  headers_add(http_client->headers, "Content-Type", "application/json");
  client_send_post(http_client, json_buffer(body), "/missions/complete");
  headers_remove(http_client->headers, "Content-Type");
  json_destroy(body);
  free(args);
  missions_refresh_async();
}

void missions_complete(int32_t mission_id) {
  Mission *missions = missions_get(MISSIONS_MANUAL);
  int index = missions_find(MISSIONS_MANUAL, mission_id);

  if (index < 0)
    return;

  Mission *item = &missions[index];
  item->completed_at = now_timestamp();

  int32_t *id = malloc(sizeof(int32_t));
  *id = mission_id;
  task_start("mission_complete", mission_complete_task, id);
}
