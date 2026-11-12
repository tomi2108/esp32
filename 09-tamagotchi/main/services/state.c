#include "state.h"

static State state;

extern MqttClient mqtt_client;

void update_state(MqttClient client, MqttMessage message) {
  t_json_value *new_state = json_parse_buffer(message.buffer);

  if (new_state == NULL) {
    mqtt_destroy_message(message);
    return;
  }

  json_value_double(json_object_get(new_state, "energy"), &state.energy);
  json_value_double(json_object_get(new_state, "hunger"), &state.hunger);
  json_value_double(json_object_get(new_state, "hydration"), &state.hydration);
  json_value_double(json_object_get(new_state, "cleanliness"),
                    &state.cleanliness);
  json_value_double(json_object_get(new_state, "happiness"), &state.happiness);
  json_value_bool(json_object_get(new_state, "is_dead"), &state.is_dead);

  const char *status = json_value_string(json_object_get(new_state, "status"));
  const char *name = json_value_string(json_object_get(new_state, "name"));

  snprintf(state.status, sizeof(state.status), "%s",
           status != NULL ? status : "idle");
  snprintf(state.name, sizeof(state.name), "%s",
           name != NULL ? name : "Anonymous");

  json_value_destroy(new_state);
  mqtt_destroy_message(message);
}

State state_get(void) { return state; }

void state_init(void) { mqtt_sub(mqtt_client, "/state", update_state); }
