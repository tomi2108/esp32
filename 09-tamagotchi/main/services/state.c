#include "state.h"
#include <string.h>

static State state;

extern MqttClient mqtt_client;

PetSpecies pet_species(const char *species) {
  if (strcmp(species, "cat") == 0)
    return PET_CAT;
  if (strcmp(species, "dog") == 0)
    return PET_DOG;
  if (strcmp(species, "rabbit") == 0)
    return PET_RABBIT;

  return PET_CAT;
}

PetStatus pet_status(const char *status) {
  if (strcmp(status, "idle") == 0)
    return PET_IDLE;
  if (strcmp(status, "sleeping") == 0)
    return PET_SLEEPING;
  return PET_IDLE;
}

static t_json_value *state_body;
void update_state(MqttClient client, MqttMessage message) {
  t_json_value *new_state = json_parse_buffer(message.buffer);
  mqtt_destroy_message(message);

  if (new_state == NULL)
    return;

  if (state_body)
    json_value_destroy(state_body);
  state_body = new_state;

  json_value_double(json_object_get(new_state, "energy"), &state.energy);
  json_value_double(json_object_get(new_state, "hunger"), &state.hunger);
  json_value_double(json_object_get(new_state, "hydration"), &state.hydration);
  json_value_double(json_object_get(new_state, "cleanliness"),
                    &state.cleanliness);
  json_value_double(json_object_get(new_state, "happiness"), &state.happiness);
  json_value_bool(json_object_get(new_state, "is_dead"), &state.is_dead);

  const char *status = json_value_string(json_object_get(new_state, "status"));
  state.name = json_value_string(json_object_get(new_state, "name"));
  const char *species =
      json_value_string(json_object_get(new_state, "species"));

  state.status = pet_status(status);
  state.species = pet_species(species);
}

State state_get(void) { return state; }

void state_init(void) { mqtt_sub(mqtt_client, "/state", update_state); }
