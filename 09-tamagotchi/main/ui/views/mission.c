#include "mission.h"
#include "services/missions.h"
#include "ui/screen.h"
#include <stdint.h>

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
static int32_t id;

typedef struct {
  int32_t id;
  MissionType type;
} ScreenCtx;

static Option missions_options[2] = {{.key = "Completar", .value = 0},
                                     {.key = "Salir", .value = 1}};

static void select_option(void *ctx, int32_t option) {
  switch (option) {
  case 0:
    missions_complete(id);
    break;
  default:
    break;
  }
  screen_pop();
  screen_pop();
}

static Menu mission_menu = {
    .options = missions_options,
    .length = 2,
    .current = 0,
    .selected = -1,
    .on_select = select_option,
    .buzzer = &buzzer,
    .context = NULL,
};

static void render(Screen *self) {
  ScreenCtx ctx = *(ScreenCtx *)self->context;
  Mission *missions = missions_get(ctx.type);
  int index = missions_find(ctx.type, ctx.id);
  Mission mission = missions[index];

  oled_write_text(&oled, 0, 0, false, "%s", mission.title);
  oled_write_text(&oled, 0, oled.font.line_height * 2, false, "%s",
                  mission.description);

  if (ctx.type == MISSIONS_MANUAL) {
    oled_write_text(&oled, 0, oled.height - 8, mission_menu.current == 0, "%s",
                    missions_options[0].key);
    oled_write_text(&oled, 80, oled.height - 8, mission_menu.current == 1, "%s",
                    missions_options[1].key);
  }
}

static void update(Screen *self) {
  ScreenCtx ctx = *(ScreenCtx *)self->context;

  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (ctx.type == MISSIONS_AUTOMATIC) {
    if (click == ROTARY_CLICK_DOUBLE)
      screen_pop();
    return;
  }

  if (ctx.type == MISSIONS_MANUAL) {
    if (click == ROTARY_CLICK_DOUBLE)
      screen_pop();

    if (click == ROTARY_CLICK_SINGLE)
      menu_select(&mission_menu);

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&mission_menu);

    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&mission_menu);
  }
}

Screen mission_screen = {
    .render = render,
    .update = update,
};

void mission_view(MissionType type, int32_t mission_id) {
  ScreenCtx *ctx = malloc(sizeof(ScreenCtx));
  id = mission_id;
  ctx->id = mission_id;
  ctx->type = type;
  mission_screen.context = ctx;
  mission_menu.current = 0;
  mission_menu.selected = -1;
  screen_push(mission_screen);
}
