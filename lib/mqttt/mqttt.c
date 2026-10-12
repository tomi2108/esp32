#include "mqttt.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct MqttSubscription {
  char *topic;
  MqttMessageCallback callback;
  struct MqttSubscription *next;
} MqttSubscription;

typedef struct {
  MqttClient client;
  bool connected;
  MqttSubscription *subscriptions;

  char *message_topic;
  t_buffer *message_buffer;
  size_t message_total;
} MqttState;

static char *mqtt_strdup(const char *string) {
  if (string == NULL)
    return NULL;

  size_t length = strlen(string) + 1;
  char *copy = malloc(length);

  if (copy != NULL)
    memcpy(copy, string, length);

  return copy;
}

static void mqtt_reset_message(MqttState *state) {
  free(state->message_topic);
  state->message_topic = NULL;

  buffer_destroy(state->message_buffer);
  state->message_buffer = NULL;

  state->message_total = 0;
}

static void mqtt_subscribe_all(MqttState *state) {
  for (MqttSubscription *sub = state->subscriptions; sub != NULL;
       sub = sub->next) {
    esp_mqtt_client_subscribe(state->client._client, sub->topic, 1);
  }
}

static void mqtt_event_handler(void *args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  MqttState *state = args;
  esp_mqtt_event_handle_t event = event_data;

  if (state == NULL || event == NULL)
    return;

  switch (event_id) {
  case MQTT_EVENT_CONNECTED:
    state->connected = true;
    mqtt_subscribe_all(state);
    break;

  case MQTT_EVENT_DISCONNECTED:
    state->connected = false;
    break;

  case MQTT_EVENT_DATA: {
    if (event->current_data_offset == 0) {
      mqtt_reset_message(state);

      if (event->topic == NULL || event->topic_len <= 0 ||
          event->total_data_len < 0 || event->data_len < 0)
        break;

      state->message_topic = malloc((size_t)event->topic_len + 1);
      state->message_buffer = buffer_create();

      if (state->message_topic == NULL || state->message_buffer == NULL) {
        mqtt_reset_message(state);
        break;
      }

      memcpy(state->message_topic, event->topic, (size_t)event->topic_len);
      state->message_topic[event->topic_len] = '\0';
      state->message_total = (size_t)event->total_data_len;
    }

    if (state->message_buffer == NULL || event->data_len < 0 ||
        (size_t)event->data_len >
            state->message_total - state->message_buffer->size) {
      mqtt_reset_message(state);
      break;
    }

    if (event->data_len > 0 && buffer_add(state->message_buffer, event->data,
                                          (size_t)event->data_len) != 0) {
      mqtt_reset_message(state);
      break;
    }

    if (state->message_buffer->size < state->message_total)
      break;

    for (MqttSubscription *sub = state->subscriptions; sub != NULL;
         sub = sub->next) {
      if (strcmp(sub->topic, state->message_topic) != 0)
        continue;

      MqttMessage message = {.buffer = buffer_create()};

      if (message.buffer == NULL)
        continue;

      if (state->message_buffer->size > 0 &&
          buffer_add(message.buffer, state->message_buffer->stream,
                     state->message_buffer->size) != 0) {
        mqtt_destroy_message(message);
        continue;
      }

      sub->callback(state->client, message);
    }

    mqtt_reset_message(state);
    break;
  }

  default:
    break;
  }
}

MqttClient mqtt_get_client(char *url, char *username, char *password) {
  MqttClient client = {0};

  if (url == NULL)
    return client;

  MqttState *state = calloc(1, sizeof(*state));

  if (state == NULL)
    return client;

  client.url = mqtt_strdup(url);

  if (client.url == NULL) {
    free(state);
    return (MqttClient){0};
  }

  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.verification.crt_bundle_attach = esp_crt_bundle_attach,
      .broker.address.uri = url,
      .credentials =
          {
              .username = username,
              .authentication.password = password,
              .client_id = username,
          },
  };

  client._client = esp_mqtt_client_init(&mqtt_cfg);

  if (client._client == NULL) {
    free(client.url);
    free(state);
    return (MqttClient){0};
  }

  client._state = state;
  state->client = client;

  if (esp_mqtt_client_register_event(client._client, ESP_EVENT_ANY_ID,
                                     mqtt_event_handler, state) != ESP_OK) {
    esp_mqtt_client_destroy(client._client);
    free(client.url);
    free(state);
    return (MqttClient){0};
  }

  if (esp_mqtt_client_start(client._client) != ESP_OK) {
    esp_mqtt_client_destroy(client._client);
    free(client.url);
    free(state);
    return (MqttClient){0};
  }

  return client;
}

int mqtt_pub(MqttClient client, const char *topic, const MqttMessage message) {
  if (client._client == NULL || topic == NULL || message.buffer == NULL ||
      message.buffer->size > INT_MAX)
    return -1;

  int message_id = esp_mqtt_client_publish(client._client, topic,
                                           (const char *)message.buffer->stream,
                                           (int)message.buffer->size, 1, 0);

  return message_id < 0 ? -1 : message_id;
}

int mqtt_sub(MqttClient client, const char *topic,
             MqttMessageCallback callback) {
  if (client._client == NULL || client._state == NULL || topic == NULL ||
      callback == NULL || topic[0] == '\0')
    return -1;

  MqttState *state = client._state;

  for (MqttSubscription *sub = state->subscriptions; sub != NULL;
       sub = sub->next) {
    if (strcmp(sub->topic, topic) == 0)
      return -1;
  }

  MqttSubscription *sub = calloc(1, sizeof(*sub));

  if (sub == NULL)
    return -1;

  sub->topic = mqtt_strdup(topic);

  if (sub->topic == NULL) {
    free(sub);
    return -1;
  }

  sub->callback = callback;
  sub->next = state->subscriptions;
  state->subscriptions = sub;

  if (state->connected) {
    int message_id = esp_mqtt_client_subscribe(client._client, topic, 1);

    if (message_id < 0) {
      state->subscriptions = sub->next;
      free(sub->topic);
      free(sub);
      return -1;
    }
  }

  return 0;
}

void mqtt_destroy_message(MqttMessage message) {
  buffer_destroy(message.buffer);
}

void mqtt_destroy_client(MqttClient client) {
  MqttState *state = client._state;

  if (client._client != NULL) {
    esp_mqtt_client_stop(client._client);
    esp_mqtt_client_destroy(client._client);
  }

  if (state != NULL) {
    mqtt_reset_message(state);

    MqttSubscription *sub = state->subscriptions;

    while (sub != NULL) {
      MqttSubscription *next = sub->next;

      free(sub->topic);
      free(sub);

      sub = next;
    }

    free(state);
  }

  free(client.url);
}
