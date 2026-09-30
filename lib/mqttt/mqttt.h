#include "driver/gpio.h"
#include "mqtt_client.h"

typedef struct MqttClient {
  char *url;
  esp_mqtt_client_handle_t _client;
} MqttClient;

typedef struct MqttMessage {
  char *text;
} MqttMessage;

typedef void (*MqttMessageCallback)(MqttClient client,
                                    const MqttMessage message);

MqttClient mqtt_get_client(char *url, char *topic);
void mqtt_on_data(MqttClient client, MqttMessageCallback handler);
