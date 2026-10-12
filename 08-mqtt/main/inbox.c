#include "./env.h"
#include "buzzer.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "rotary_encoder.h"

#define INBOX_SIZE 500

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
Menu inbox_menu;

typedef struct Message {
  char *message;
  bool is_read;
} Message;

Message inbox[INBOX_SIZE] = {0};
bool showing_message = false;

int inbox_count() {
  int c = 0;
  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].message != NULL)
      c++;
  }
  return c;
}

void inbox_add(Message m) {
  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].message == NULL) {
      inbox[i].message = m.message;
      inbox[i].is_read = m.is_read;
      return;
    }
  }
};

void inbox_read(int index) { inbox[index].is_read = true; };

void inbox_remove(int index) {
  free(inbox[index].message);
  inbox[index].message = NULL;
};

int inbox_get_unread() {
  int unread = 0;
  for (int i = 0; i < INBOX_SIZE; i++) {
    Message m = inbox[i];
    if (m.message != NULL && !m.is_read)
      unread++;
  }
  return unread;
};

void message_recieved(MqttClient client, MqttMessage message) {
  Message m = {.message = buffer_read_string(message.buffer), .is_read = false};
  inbox_add(m);
  mqtt_destroy_message(message);
  buzzer_play_melody(buzzer, wholenote, sound_notification());
}

void message_display(Message m) {
  oled_clear(&oled);
  if (m.message != NULL)
    oled_write_text(&oled, 0, 0, false, m.message);
  oled_update(&oled);
};

void select_message(void *context, int32_t index) {
  showing_message = true;
  message_display(inbox[index]);
};

void inbox_display() {

  static Option o[INBOX_SIZE];

  int j = 0;

  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].message != NULL) {
      o[j] = (Option){
          .key = inbox[i].message,
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

void mqtt_connect() {
  MqttClient client =
      mqtt_get_client(MQTT_URL, MQTT_USERNAME, MQTT_PASSWORD, MQTT_TOPIC);
  mqtt_on_data(client, message_recieved);
}

void inbox_on_boot() {
  mqtt_connect();
  inbox_menu.buzzer = &buzzer;
  inbox_menu.on_select = select_message;
}

void inbox_update() {
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

      inbox_display();
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
