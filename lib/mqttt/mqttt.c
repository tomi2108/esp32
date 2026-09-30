#include "mqttt.h"

void subscribe_handler(void *handler_args, esp_event_base_t base,
                       int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  char *topic = handler_args;
  esp_mqtt_client_subscribe(event->client, topic, 1);
}

Mqtt_Client mqtt_get_client(char *url, char *topic) {
  Mqtt_Client client = {.url = url};
  esp_mqtt_client_config_t mqtt_cfg = {.broker.address.uri = url};
  client._client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client._client);
  esp_mqtt_client_register_event(client._client, MQTT_EVENT_CONNECTED,
                                 subscribe_handler, topic);
  return client;
};

void mqtt_subscribe(Mqtt_Client client, esp_mqtt_event_id_t event,
                    esp_event_handler_t handler) {
  esp_mqtt_client_register_event(client._client, event, handler, NULL);
}
