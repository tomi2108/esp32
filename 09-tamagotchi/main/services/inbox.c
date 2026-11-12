#include "inbox.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

Message inbox[INBOX_SIZE] = {0};

Message inbox_get(int index) { return inbox[index]; }

int inbox_count(void) {
  int count = 0;
  for (int i = 0; i < INBOX_SIZE; i++)
    if (inbox[i].content[0] != '\0')
      count++;
  return count;
}

int inbox_get_unread(void) {
  int unread = 0;
  for (int i = 0; i < INBOX_SIZE; i++)
    if (inbox[i].content[0] != '\0' && !inbox[i].is_read)
      unread++;
  return unread;
}

void inbox_remove(int index) {
  if (index < 0 || index >= INBOX_SIZE)
    return;
  inbox[index].content[0] = '\0';
}

void inbox_add(Message message) {
  for (int i = INBOX_SIZE - 1; i > 0; i--)
    inbox[i] = inbox[i - 1];
  inbox[0] = message;
}

static void decode_message(const t_json_value *message, Message *out) {
  if (message == NULL || out == NULL)
    return;

  const char *id = json_value_string(json_object_get(message, "id"));
  const char *content = json_value_string(json_object_get(message, "content"));
  const char *read_at = json_value_string(json_object_get(message, "read_at"));

  snprintf(out->id, sizeof(out->id), "%s", id != NULL ? id : "");
  snprintf(out->content, sizeof(out->content), "%s",
           content != NULL ? content : "");

  out->is_read = read_at != NULL;
}

void inbox_fetch(void) {
  char path[32];
  snprintf(path, sizeof(path), "/inbox/%d", PLAYER_ID);
  Response *res = client_send_get(http_client, path);
  if (res == NULL)
    return;
  if (res->status_code < 200 || res->status_code >= 300) {
    response_destroy(res);
    return;
  }

  t_json_value *inbox_res = json_parse_buffer(res->body);
  if (inbox_res == NULL) {
    response_destroy(res);
    return;
  }
  memset(inbox, 0, sizeof(inbox));
  int count = json_array_size(inbox_res);
  if (count > INBOX_SIZE)
    count = INBOX_SIZE;
  for (int i = 0; i < count; i++) {
    const t_json_value *message = json_array_get(inbox_res, i);
    if (message != NULL)
      decode_message(message, &inbox[i]);
  }
  json_value_destroy(inbox_res);
  response_destroy(res);
}

static void mark_as_read_task(void *arg) {
  char *path = arg;
  client_send_patch(http_client, path);
}

static void mark_as_read(const char *id) {
  if (id == NULL)
    return;
  char *path = malloc(128);
  if (path == NULL)
    return;
  snprintf(path, 128, "/message/%s/read?", id);
  if (task_start("mark_as_read", mark_as_read_task, path) != 0)
    free(path);
}

void inbox_read(int index) {
  if (index < 0 || index >= INBOX_SIZE || inbox[index].content[0] == '\0' ||
      inbox[index].is_read)
    return;
  mark_as_read(inbox[index].id);
  inbox[index].is_read = true;
}

static void message_recieved(MqttClient client, MqttMessage message) {
  t_json_value *body = json_parse_buffer(message.buffer);
  if (body == NULL) {
    mqtt_destroy_message(message);
    return;
  }
  const t_json_value *id = json_object_get(body, "id");
  const t_json_value *content = json_object_get(body, "content");
  if (id != NULL && content != NULL) {
    Message message_data = {0};
    decode_message(body, &message_data);
    if (message_data.content[0] != '\0') {
      inbox_add(message_data);
      buzzer_play_melody(buzzer, wholenote, sound_notification());
    }
  }
  json_value_destroy(body);
  mqtt_destroy_message(message);
}

void inbox_init(void) {
  char topic[32];
  snprintf(topic, sizeof(topic), "/message/%d", PLAYER_ID);
  mqtt_sub(mqtt_client, topic, message_recieved);
  inbox_fetch();
}
