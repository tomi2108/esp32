#include "buffer.h"
#include "driver/gpio.h"
#include "esp_crt_bundle.h"
#include "mqtt_client.h"

typedef struct MqttClient {
  char *url;
  esp_mqtt_client_handle_t _client;
} MqttClient;

typedef struct MqttMessage {
  t_buffer *buffer;
} MqttMessage;

typedef void (*MqttMessageCallback)(MqttClient client,
                                    const MqttMessage message);

MqttClient mqtt_get_client(char *url, char *username, char *password,
                           char *topic);
void mqtt_on_data(MqttClient client, MqttMessageCallback handler);
void mqtt_destroy_message(MqttMessage message);
