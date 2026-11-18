#include "inbox.h"
#include "json.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

Message inbox[INBOX_SIZE] = {0};
int inbox_size = 0;

Message inbox_get_message(int index) { return inbox[index]; }
Message *inbox_get() { return inbox; }

int inbox_get_size(void) { return inbox_size; }

int inbox_get_unread(void) {
  int unread = 0;
  for (int i = 0; i < INBOX_SIZE; i++)
    if (inbox[i].content[0] != '\0' && !inbox[i].is_read)
      unread++;
  return unread;
}

static void inbox_remove(int index) {
  Message *inbox = inbox_get();
  int size = inbox_get_size();
  if (index < 0 || index >= size)
    return;
  for (int i = index; i < size - 1; i++)
    inbox[i] = inbox[i + 1];
  inbox_size--;
}

void inbox_add(Message message) {
  for (int i = INBOX_SIZE - 1; i > 0; i--)
    inbox[i] = inbox[i - 1];
  inbox[0] = message;
  inbox_size++;
}

static void decode_message(const t_json_value *message, Message *out) {
  if (message == NULL || out == NULL)
    return;

  out->id = json_value_string(json_object_get(message, "id"));
  out->content = json_value_string(json_object_get(message, "content"));
  const char *read_at = json_value_string(json_object_get(message, "read_at"));
  out->is_read = read_at != NULL;
}

static t_json_value *inbox_body;
void inbox_fetch(void) {
  char path[32];
  snprintf(path, sizeof(path), "/inbox/%d", PLAYER_ID);
  Response *res = client_send_get(http_client, path);

  if (res == NULL)
    return;

  t_json_value *body = json_parse_buffer(res->body);
  response_destroy(res);

  if (body == NULL)
    return;

  if (inbox_body)
    json_value_destroy(inbox_body);
  inbox_body = body;

  inbox_size = 0;
  int count = json_array_size(body);
  if (count > INBOX_SIZE)
    count = INBOX_SIZE;
  for (int i = 0; i < inbox_size; i++) {
    const t_json_value *message = json_array_get(body, i);
    if (message != NULL) {
      decode_message(message, &inbox[i]);
      inbox_size++;
    }
  }
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
  mqtt_destroy_message(message);
  if (body == NULL)
    return;

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
}

void inbox_init(void) {
  char topic[32];
  snprintf(topic, sizeof(topic), "/message/%d", PLAYER_ID);
  mqtt_sub(mqtt_client, topic, message_recieved);
  inbox_fetch();
}
