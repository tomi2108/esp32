#include "inbox.h"

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

static Menu inbox_menu;
static int selected_message = -1;

static const OLEDMenuHeader header = {
    .inverted = true,
    .separator = true,
    .title = "Mensajitos",
};

static void message_render(Screen *self) {
  if (selected_message < 0 || selected_message >= INBOX_SIZE)
    return;
  Message message = inbox_get(selected_message);
  if (message.content[0] != '\0')
    oled_write_text(&oled, 0, 0, false, "%s", message.content);
}

static void message_update(Screen *self) {
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (click == ROTARY_CLICK_DOUBLE) {
    screen_pop();
    return;
  }
}

static Screen message_screen = {
    .render = message_render,
    .update = message_update,
};

static void select_message(void *context, int32_t index) {
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
    extern Message inbox[INBOX_SIZE];
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

static Screen inbox_screen = {
    .render = inbox_list_render,
    .update = inbox_list_update,
};

void inbox_display(void) {
  inbox_menu.current = 0;
  inbox_menu.selected = -1;
  inbox_menu.buzzer = &buzzer;
  inbox_menu.on_select = select_message;
  selected_message = -1;
  screen_push(inbox_screen);
}
