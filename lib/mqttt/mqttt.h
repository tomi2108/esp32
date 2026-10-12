#ifndef MQTT_H
#define MQTT_H

#include "buffer.h"
#include "esp_crt_bundle.h"
#include "mqtt_client.h"

typedef struct MqttClient {
  char *url;
  esp_mqtt_client_handle_t _client;
  void *_state;
} MqttClient;

typedef struct MqttMessage {
  t_buffer *buffer;
} MqttMessage;

typedef void (*MqttMessageCallback)(MqttClient client,
                                    const MqttMessage message);

MqttClient mqtt_get_client(char *url, char *username, char *password);

int mqtt_pub(MqttClient client, const char *topic, const MqttMessage message);

int mqtt_sub(MqttClient client, const char *topic,
             MqttMessageCallback callback);

void mqtt_destroy_client(MqttClient client);
void mqtt_destroy_message(MqttMessage message);

#endif
