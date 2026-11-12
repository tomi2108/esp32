#include "inbox.h"
#include "icons.h"
#include "oled.h"
#include "screen.h"
#include "task.h"
#include "toolbar.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

static Menu inbox_menu;
static int selected_message = -1;

static const OLEDMenuHeader header = {
    .inverted = true,
    .separator = true,
    .title = "Mensajitos",
};

Message inbox[INBOX_SIZE] = {0};

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

void inbox_fetch(void) {
  char path[32];
  snprintf(path, sizeof(path), "/inbox/%d", PLAYER_ID);
  Response *res = client_send_get(http_client, path);
  if (res == NULL)
    return;
  if (res->status_code < 200 || res->status_code >= 300) {
    response_destroy(res);
    return;
  }

  t_json_value *inbox_res = json_parse_buffer(res->body);
  if (inbox_res == NULL) {
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

static void mark_as_read_task(void *arg) {
  char *path = arg;
  client_send_patch(http_client, path);
}

void mark_as_read(const char *id) {
  if (id == NULL)
    return;
  char *path = malloc(128);
  if (path == NULL)
    return;
  snprintf(path, 128, "/message/%s/read?", id);
  if (task_start("mark_as_read", mark_as_read_task, path) != 0)
    free(path);
}

void inbox_read(int index) {
  if (index < 0 || index >= INBOX_SIZE || inbox[index].content[0] == '\0' ||
      inbox[index].is_read)
    return;
  mark_as_read(inbox[index].id);
  inbox[index].is_read = true;
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
    if (m.content[0] != '\0') {
      inbox_add(m);
      buzzer_play_melody(buzzer, wholenote, sound_notification());
    }
  }
  json_value_destroy(body);
  mqtt_destroy_message(message);
}

static void message_render(Screen *self) {
  if (selected_message < 0 || selected_message >= INBOX_SIZE)
    return;
  Message *message = &inbox[selected_message];
  if (message->content[0] != '\0')
    oled_write_text(&oled, 0, 0, false, "%s", message->content);
}

static void message_update(Screen *self) {
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (click == ROTARY_CLICK_DOUBLE) {
    screen_pop();
    return;
  }
}

Screen message_screen = {
    .render = message_render,
    .update = message_update,
};

void select_message(void *context, int32_t index) {
  if (index < 0 || index >= inbox_menu.length)
    return;

  int inbox_index = inbox_menu.options[index].value;

  if (inbox_index < 0 || inbox_index >= INBOX_SIZE)
    return;
  inbox_read(inbox_index);
  selected_message = inbox_index;
  screen_push(message_screen);
}

static void inbox_list_render(Screen *self) {
  int count = inbox_count();
  if (count == 0) {
    oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Inbox empty");
    return;
  }

  if (inbox_menu.current >= count)
    inbox_menu.current = count - 1;
  static Option options[INBOX_SIZE];
  static OLEDMenuItemDecoration decorations[INBOX_SIZE];
  int j = 0;
  for (int i = 0; i < INBOX_SIZE; i++) {
    if (inbox[i].content[0] == '\0')
      continue;
    options[j] = (Option){
        .key = (char *)inbox[i].content,
        .value = i,
    };

    decorations[j] = (OLEDMenuItemDecoration){
        .label = NULL,
        .bitmap = inbox[i].is_read
                      ? icons_software_email_message_read_opened()
                      : icons_software_email_message_unread_closed(),
    };
    j++;
  }

  inbox_menu.on_select = select_message;
  inbox_menu.length = j;
  inbox_menu.options = options;

  const OLEDMenu menu = {
      .header = &header,
      .bitmap_height = 16,
      .bitmap_width = 16,
      .decorations = decorations,
      .menu = inbox_menu,
  };

  oled_display_menu(&oled, &menu);
}

static void inbox_list_update(Screen *self) {
  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);
  int count = inbox_count();
  if (click == ROTARY_CLICK_DOUBLE) {
    screen_pop();
    return;
  }

  if (count == 0)
    return;

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&inbox_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&inbox_menu);

  if (click == ROTARY_CLICK_SINGLE) {
    rotary_encoder_wait_for_release(&rotary_encoder);
    menu_select(&inbox_menu);
  }
}

Screen inbox_screen = {
    .render = inbox_list_render,
    .update = inbox_list_update,
};

void inbox_init(void) {
  char topic[32];

  snprintf(topic, sizeof(topic), "/message/%d", PLAYER_ID);

  mqtt_sub(mqtt_client, topic, message_recieved);

  inbox_menu.buzzer = &buzzer;
  inbox_menu.current = 0;
  inbox_menu.selected = -1;
  inbox_menu.on_select = select_message;

  inbox_fetch();
}

void inbox_display(void) {
  inbox_menu.current = 0;
  inbox_menu.selected = -1;
  selected_message = -1;
  screen_push(inbox_screen);
}
