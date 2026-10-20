#include "inbox.h"
#include "icons.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

Menu inbox_menu;

Message inbox[INBOX_SIZE] = {0};
bool showing_message = false;

int inbox_count(void) {
  int count = 0;

  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content[0] != '\0')
      count++;
  }

  return count;
}

int inbox_get_unread(void) {
  int unread = 0;

  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content[0] != '\0' && !inbox[i].is_read)
      unread++;
  }

  return unread;
}

void inbox_remove(int index) {
  if (index < 0 || index >= INBOX_SIZE)
    return;

  inbox[index].content[0] = '\0';
}

void inbox_add(Message m) {
  for (int i = INBOX_SIZE - 1; i > 0; i--)
    inbox[i] = inbox[i - 1];

  inbox[0] = m;
}

static void mark_as_read_task(void *arg) {
  char *path = arg;
  Response *res = client_send_patch(http_client, path);

  if (res == NULL) {
    ESP_LOGE("inbox", "Failed to mark message as read: request failed");
  } else {
    if (res->status_code < 200 || res->status_code >= 300)
      ESP_LOGE("inbox", "Failed to mark message as read: HTTP %d",
               res->status_code);
    response_destroy(res);
  }

  free(path);
  vTaskDelete(NULL);
}

void mark_as_read(const char *id) {
  if (id == NULL)
    return;
  char *path = malloc(128);
  if (path == NULL)
    return;
  snprintf(path, 128, "/message/%s/read?", id);
  if (xTaskCreate(mark_as_read_task, "mark_as_read", 4096, path, 5, NULL) !=
      pdPASS) {
    free(path);
    ESP_LOGE("inbox", "Failed to create mark-as-read task");
  }
}

void inbox_read(int index) {
  if (index < 0 || index >= INBOX_SIZE || inbox[index].content[0] == '\0' ||
      inbox[index].is_read)
    return;

  mark_as_read(inbox[index].id);
  inbox[index].is_read = true;
}

void decode_message(const t_json_value *message, Message *out) {
  if (message == NULL || out == NULL)
    return;

  const char *id = json_value_string(json_object_get(message, "id"));
  const char *content = json_value_string(json_object_get(message, "content"));
  const char *read_at = json_value_string(json_object_get(message, "read_at"));

  snprintf(out->id, sizeof(out->id), "%s", id != NULL ? id : "");
  snprintf(out->content, sizeof(out->content), "%s",
           content != NULL ? content : "");
  out->is_read = read_at != NULL;
}

void message_recieved(MqttClient client, MqttMessage message) {
  t_json_value *body = json_parse_buffer(message.buffer);
  if (body == NULL) {
    mqtt_destroy_message(message);
    return;
  }
  const t_json_value *id = json_object_get(body, "id");
  const t_json_value *content = json_object_get(body, "content");

  if (id != NULL && content != NULL) {
    Message m = {0};
    decode_message(body, &m);
    if (m.content[0] != '\0')
      inbox_add(m);
    buzzer_play_melody(buzzer, wholenote, sound_notification());
  }

  json_value_destroy(body);
  mqtt_destroy_message(message);
}

void message_display(Message m) {
  oled_clear(&oled);

  if (m.content[0] != '\0')
    oled_write_text(&oled, 0, 0, false, "%s", m.content);

  oled_update(&oled);
}

void select_message(void *context, int32_t index) {
  if (index < 0 || index >= inbox_menu.length)
    return;
  int inbox_index = inbox_menu.options[index].value;
  if (inbox_index < 0 || inbox_index >= INBOX_SIZE)
    return;
  inbox_read(inbox_index);
  showing_message = true;
  message_display(inbox[inbox_index]);
}

void inbox_menu_display() {
  static Option o[INBOX_SIZE];
  static Icon icons[INBOX_SIZE];

  int j = 0;

  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content[0] == '\0')
      continue;

    o[j] = (Option){
        .key = (char *)inbox[i].content,
        .value = i,
    };

    icons[j] = inbox[i].is_read ? icons_software_email_message_read_opened()
                                : icons_software_email_message_unread_closed();
    j++;
  }

  inbox_menu.on_select = select_message;
  inbox_menu.length = j;
  inbox_menu.options = o;

  oled_display_menu_with_bitmaps(&oled, inbox_menu, icons, NULL, ICON_HEIGHT,
                                 ICON_WIDTH);
  oled_update(&oled);
}

void inbox_fetch(void) {
  char path[32];
  snprintf(path, sizeof(path), "/inbox/%d", RECIPIENT_ID);

  Response *res = client_send_get(http_client, path);
  if (res == NULL)
    return;

  if (res->status_code < 200 || res->status_code >= 300) {
    ESP_LOGE("inbox", "Failed to fetch inbox: HTTP %d", res->status_code);
    response_destroy(res);
    return;
  }

  t_json_value *inbox_res = json_parse_buffer(res->body);

  if (inbox_res == NULL) {
    ESP_LOGE("inbox", "Failed to parse inbox response");
    response_destroy(res);
    return;
  }

  memset(inbox, 0, sizeof(inbox));

  int count = json_array_size(inbox_res);
  if (count > INBOX_SIZE)
    count = INBOX_SIZE;

  for (int i = 0; i < count; i++) {
    const t_json_value *message = json_array_get(inbox_res, i);
    if (message != NULL)
      decode_message(message, &inbox[i]);
  }

  json_value_destroy(inbox_res);
  response_destroy(res);
}

void inbox_init() {
  char topic[32];
  snprintf(topic, sizeof(topic), "/message/%d", RECIPIENT_ID);
  mqtt_sub(mqtt_client, topic, message_recieved);
  inbox_menu.buzzer = &buzzer;
  inbox_menu.on_select = select_message;
  inbox_fetch();
}

void inbox_display() {
  bool back = false;

  while (!back) {
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_DOUBLE) {
      back = true;
      continue;
    }

    if (!showing_message) {
      int count = inbox_count();
      if (count == 0) {
        oled_clear(&oled);
        oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Inbox empty");
        oled_update(&oled);
        vTaskDelay(pdMS_TO_TICKS(10));
        continue;
      }

      inbox_menu_display();

      if (direction == ROTARY_CLOCKWISE)
        menu_next(&inbox_menu);

      if (direction == ROTARY_COUNTERCLOCKWISE)
        menu_prev(&inbox_menu);

      if (click == ROTARY_CLICK_SINGLE) {
        rotary_encoder_wait_for_release(&rotary_encoder);
        menu_select(&inbox_menu);
      }
    } else {
      if (click == ROTARY_CLICK_SINGLE) {
        rotary_encoder_wait_for_release(&rotary_encoder);
        showing_message = false;
      }
    }
  }
  showing_message = false;
}
