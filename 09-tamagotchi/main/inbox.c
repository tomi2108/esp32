#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define INBOX_SIZE 500

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

Menu inbox_menu;

typedef struct Message {
  const char *content;
  bool is_read;
} Message;

Message inbox[INBOX_SIZE] = {0};
bool showing_message = false;

int inbox_count() {
  int c = 0;
  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content != NULL)
      c++;
  }
  return c;
}

void inbox_add(Message m) {
  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content == NULL) {
      inbox[i].content = m.content;
      inbox[i].is_read = m.is_read;
      return;
    }
  }
};

void inbox_read(int index) { inbox[index].is_read = true; };

void inbox_remove(int index) { inbox[index].content = NULL; };

int inbox_get_unread() {
  int unread = 0;
  for (int i = 0; i < INBOX_SIZE; i++) {
    Message m = inbox[i];
    if (m.content != NULL && !m.is_read)
      unread++;
  }
  return unread;
};

void decode_message(const t_json_value *message, Message *out) {
  out->content = json_value_string(json_object_get(message, "content"));
  const char *read_at = json_value_string(json_object_get(message, "read_at"));
  if (read_at == NULL)
    out->is_read = false;
  else
    out->is_read = true;
}

void message_recieved(MqttClient client, MqttMessage message) {
  t_json_value *body = json_parse_buffer(message.buffer);
  Message m;
  decode_message(body, &m);
  inbox_add(m);
  mqtt_destroy_message(message);
  buzzer_play_melody(buzzer, wholenote, sound_notification());
}

void message_display(Message m) {
  oled_clear(&oled);
  if (m.content != NULL)
    oled_write_text(&oled, 0, 0, false, m.content);
  oled_update(&oled);
};

void select_message(void *context, int32_t index) {
  showing_message = true;
  message_display(inbox[index]);
};

void inbox_menu_display() {
  static Option o[INBOX_SIZE];

  int j = 0;

  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content != NULL) {
      o[j] = (Option){
          .key = (char *)inbox[i].content,
          .value = i,
      };
      j++;
    }
  }

  inbox_menu.on_select = select_message;
  inbox_menu.length = inbox_count();
  inbox_menu.options = o;
  oled_display_menu(&oled, inbox_menu);
  oled_update(&oled);
}

void inbox_fetch() {
  Response *res = client_send_get(http_client, "/inbox/" RECIPIENT_ID);
  t_json_value *inbox_res = json_parse_buffer(res->body);

  for (int i = 0; i < json_array_size(inbox_res); i++) {
    const t_json_value *message = json_array_get(inbox_res, i);
    decode_message(message, &inbox[i]);
  };
}

void inbox_init() {
  mqtt_sub(mqtt_client, "/message/" RECIPIENT_ID, message_recieved);
  inbox_menu.buzzer = &buzzer;
  inbox_menu.on_select = select_message;
  inbox_fetch();
}

void inbox_display() {
  // TODO: add a way to go back to menu
  while (1) {
    if (!showing_message) {
      int count = inbox_count();
      if (count == 0) {
        oled_clear(&oled);
        oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Inbox empty");
        oled_update(&oled);
        continue;
      }

      inbox_menu_display();
      RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);

      if (direction == ROTARY_CLOCKWISE)
        menu_next(&inbox_menu);

      if (direction == ROTARY_COUNTERCLOCKWISE)
        menu_prev(&inbox_menu);

      if (rotary_encoder_is_pressed(&rotary_encoder)) {
        rotary_encoder_wait_for_release(&rotary_encoder);
        menu_select(&inbox_menu);
      }
    } else {
      if (rotary_encoder_is_pressed(&rotary_encoder)) {
        rotary_encoder_wait_for_release(&rotary_encoder);
        showing_message = false;
      }
    }
  }
}
