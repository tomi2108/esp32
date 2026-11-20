#include "missions.h"
#include "services/missions.h"

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

static char missions_labels[MAX_MISSIONS_SIZE][64];
static char missions_reward[MAX_MISSIONS_SIZE][12];
static Option missions_options[MAX_MISSIONS_SIZE];
static OLEDMenuItemDecoration missions_decorations[MAX_MISSIONS_SIZE];

static Menu missions_menu = {
    .options = missions_options,
    .length = 0,
    .current = 0,
    .selected = -1,
    .buzzer = &buzzer,
    .context = NULL,
};

static void update_options(MissionType type) {
  Mission *missions = missions_get(type);
  int size = missions_get_size(type);
  int count = 0;

  for (int i = 0; i < size; i++) {
    Mission mission = missions[i];

    if (mission.completed_at != NULL)
      continue;

    snprintf(missions_labels[count], sizeof(missions_labels[count]), "%s",
             mission.title ? mission.title : "mission");

    snprintf(missions_reward[count], sizeof(missions_reward[count]), "%" PRId32,
             mission.reward_coins);

    missions_options[count] = (Option){
        .key = missions_labels[count],
        .value = mission.id,
    };

    missions_decorations[count] = (OLEDMenuItemDecoration){
        .bitmap = icons_rpg_coin_gold_currency_money_gp(),
        .label = missions_reward[count],
    };

    count++;
  }

  missions_menu.length = count;

  if (missions_menu.current >= count)
    missions_menu.current = count > 0 ? count - 1 : 0;
}

void reset_time_render(void) {}

static void render(Screen *self) {
  MissionType type = *(MissionType *)self->context;
  int size = missions_menu.length;

  char *title = "";
  switch (type) {
  case MISSIONS_MANUAL:
    title = "Tareas";
    break;
  case MISSIONS_AUTOMATIC:
    title = "Misiones";
    break;
  }

  const OLEDMenuHeader missions_header = {
      .title = title,
      .inverted = true,
      .separator = true,
  };

  const OLEDMenu menu_config = {
      .menu = missions_menu,
      .header = &missions_header,
      .decorations = missions_decorations,
      .bitmap_width = 16,
      .bitmap_height = 16,
  };

  if (size == 0)
    return oled_write_text(&oled, 0, oled.height / 2 - 3, false, "No hay %s",
                           title);

  oled_display_menu(&oled, &menu_config);
  reset_time_render();
  return;
}

static void update(Screen *self) {
  MissionType type = *(MissionType *)self->context;
  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);
  int size = missions_menu.length;

  if (click == ROTARY_CLICK_DOUBLE)
    screen_pop();

  if (size == 0)
    return;

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&missions_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&missions_menu);

  if (click == ROTARY_CLICK_SINGLE && missions_menu.current >= 0 &&
      missions_menu.current < size) {
    mission_view(type, missions_menu.options[missions_menu.current].value);
  }
}

Screen missions_screen = {
    .render = render,
    .update = update,
};

void missions_view(MissionType type) {
  missions_refresh_async();
  update_options(type);
  MissionType *ctx = malloc(sizeof(MissionType));
  *ctx = type;
  missions_screen.context = ctx;
  missions_menu.current = 0;
  missions_menu.selected = -1;
  screen_push(missions_screen);
}
