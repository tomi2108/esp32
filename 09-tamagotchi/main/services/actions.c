#include "actions.h"

extern MqttClient mqtt_client;

char *topic = "/actions";

void send_action(void *data) {
  MqttMessage *message = (MqttMessage *)data;
  mqtt_pub(mqtt_client, topic, *message);
}

void action_send(t_json *body) {
  t_buffer *buffer = json_buffer(body);
  MqttMessage *message = malloc(sizeof(MqttMessage));
  message->buffer = buffer;
  const char *type =
      json_value_string(json_object_get(json_parse_buffer(buffer), "type"));
  task_start(type, send_action, message);
}

void action_sleep() {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "type");
  json_string(body, "sleep");

  json_object_end(body);
  action_send(body);
}

void action_wakeup() {
  t_json *body = json_create();
  json_object_start(body);
  json_key(body, "type");
  json_string(body, "wakeup");
  json_object_end(body);
  action_send(body);
}

void action_shower() {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "type");
  json_string(body, "shower");

  json_object_end(body);
  action_send(body);
}

void action_hydrate() {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "type");
  json_string(body, "hydrate");

  json_object_end(body);
  action_send(body);
}

void action_consume(int item_id) {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "type");
  json_string(body, "consume");

  json_key(body, "itemId");
  json_int(body, item_id);

  json_object_end(body);
  action_send(body);
}

void action_equip(int item_id) {
  t_json *body = json_create();
  json_object_start(body);

  json_key(body, "type");
  json_string(body, "equip");

  json_key(body, "itemId");
  json_int(body, item_id);

  json_object_end(body);
  action_send(body);
}
