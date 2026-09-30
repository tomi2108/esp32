#include "mqttt.h"

void subscribe_handler(void *handler_args, esp_event_base_t base,
                       int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  char *topic = handler_args;
  esp_mqtt_client_subscribe(event->client, topic, 1);
}

MqttClient mqtt_get_client(char *url, char *topic) {
  MqttClient client = {.url = url};
  esp_mqtt_client_config_t mqtt_cfg = {.broker.address.uri = url};
  client._client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client._client);
  esp_mqtt_client_register_event(client._client, MQTT_EVENT_CONNECTED,
                                 subscribe_handler, topic);
  return client;
};

void mqtt_subscribe(MqttClient client, esp_mqtt_event_id_t event,
                    esp_event_handler_t handler) {
  esp_mqtt_client_register_event(client._client, event, handler, NULL);
}

typedef struct {
  MqttMessageCallback callback;
  MqttClient client;
} __OnDataHandlerArgs;

static __OnDataHandlerArgs on_data_args = {0};
void on_data_handler(void *args, esp_event_base_t base, int32_t id,
                     void *event_data) {
  esp_mqtt_event_handle_t event = event_data;

  char data[32] = {0};
  memcpy(data, event->data, event->data_len);
  MqttMessage message = {.buffer = buffer_create()};
  buffer_add(message.buffer, event->data, event->data_len);
  on_data_args.callback(on_data_args.client, message);
}

void mqtt_destroy_message(MqttMessage message) {
  buffer_destroy(message.buffer);
}

void mqtt_on_data(MqttClient client, MqttMessageCallback callback) {
  on_data_args = (__OnDataHandlerArgs){.callback = callback, .client = client};
  esp_mqtt_client_register_event(client._client, MQTT_EVENT_DATA,
                                 on_data_handler, NULL);
}
