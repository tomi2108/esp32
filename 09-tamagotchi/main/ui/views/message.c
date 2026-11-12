#include "inbox.h"

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

static int selected_message = -1;

static void render(Screen *self) {
  if (selected_message < 0 || selected_message >= INBOX_SIZE)
    return;
  Message message = inbox_get(selected_message);
  if (message.content[0] != '\0')
    oled_write_text(&oled, 0, 0, false, "%s", message.content);
}

static void update(Screen *self) {
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (click == ROTARY_CLICK_DOUBLE) {
    screen_pop();
    return;
  }
}

static Screen message_screen = {
    .render = render,
    .update = update,
};

void message_view(int index) {
  selected_message = index;
  screen_push(message_screen);
}
