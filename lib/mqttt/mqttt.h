#include "driver/gpio.h"
#include "mqtt_client.h"

typedef struct Mqtt_Client {
  char *url;
  esp_mqtt_client_handle_t _client;
} Mqtt_Client;

Mqtt_Client mqtt_get_client(char *url, char *topic);
void mqtt_subscribe(Mqtt_Client client, esp_mqtt_event_id_t event,
                    esp_event_handler_t handler);
