#include "tictactoe.h"

#define CELL_SIZE 20
#define PADDING_X 34
#define PADDING_Y 2

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

typedef enum { NONE, PLAYER_1, PLAYER_PC } CellValue;

typedef enum {
  GAME_ONGOING,
  GAME_PLAYER_WON,
  GAME_PC_WON,
  GAME_DRAW
} GameResult;

typedef enum {
  TTT_PLAYING,
  TTT_GAME_OVER,
} TTTState;

static CellValue board[3][3];
static int cursor_row;
static int cursor_col;
static TTTState game_state;

static volatile int32_t earned_coins;

static GameResult final_result;

static void game_over_menu_select(void *ctx, int32_t selected);
static void game_over_render(Overlay *self);
static void game_over_update(Overlay *self);

static Overlay game_over_overlay = {
    .render = game_over_render,
    .update = game_over_update,
};

int col_to_x(int col) { return PADDING_X + col * CELL_SIZE; }
int row_to_y(int row) { return PADDING_Y + row * CELL_SIZE; }

static void restart(void) {
  memset(board, NONE, sizeof(board));
  cursor_row = 0;
  cursor_col = 0;
  game_state = TTT_PLAYING;
}

static void draw_player_one(int x, int y, bool on) {
  int padding = 5;
  oled_draw_line(&oled, x + padding, y + padding, x + CELL_SIZE - padding,
                 y + CELL_SIZE - padding, on);
  oled_draw_line(&oled, x + CELL_SIZE - padding, y + padding, x + padding,
                 y + CELL_SIZE - padding, on);
}

static void draw_player_two(int x, int y, bool on) {
  oled_draw_circle(&oled, x + CELL_SIZE / 2, y + CELL_SIZE / 2, 5, 7, on);
}

static void draw_cell(int row, int col, bool on) {
  int x = col_to_x(col);
  int y = row_to_y(row);
  switch (board[row][col]) {
  case NONE:
    break;

  case PLAYER_1:
    draw_player_one(x, y, on);
    break;

  case PLAYER_PC:
    draw_player_two(x, y, on);
    break;
  }
}

static void draw_board(void) {
  for (int row = 1; row < 3; row++) {
    int y = row_to_y(row);
    oled_draw_line(&oled, PADDING_X, y, PADDING_X + CELL_SIZE * 3, y, true);
  }

  for (int col = 1; col < 3; col++) {
    int x = col_to_x(col);
    oled_draw_line(&oled, x, PADDING_Y, x, PADDING_Y + CELL_SIZE * 3, true);
  }

  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      draw_cell(row, col, true);
}

static int calculate_score(void) {
  int pieces = 0;
  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      if (board[row][col] != NONE)
        pieces++;

  int score = 1000 - ((pieces - 5) * 200);
  if (score < 0)
    score = 0;
  if (score > 1000)
    score = 1000;
  return score;
}

static GameResult check_end(void) {
  static const int wins[8][3][2] = {
      {{0, 0}, {0, 1}, {0, 2}}, {{1, 0}, {1, 1}, {1, 2}},
      {{2, 0}, {2, 1}, {2, 2}}, {{0, 0}, {1, 0}, {2, 0}},
      {{0, 1}, {1, 1}, {2, 1}}, {{0, 2}, {1, 2}, {2, 2}},
      {{0, 0}, {1, 1}, {2, 2}}, {{0, 2}, {1, 1}, {2, 0}},
  };

  for (int i = 0; i < 8; i++) {
    CellValue value = board[wins[i][0][0]][wins[i][0][1]];
    if (value != NONE && value == board[wins[i][1][0]][wins[i][1][1]] &&
        value == board[wins[i][2][0]][wins[i][2][1]])
      return value == PLAYER_1 ? GAME_PLAYER_WON : GAME_PC_WON;
  }

  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      if (board[row][col] == NONE)
        return GAME_ONGOING;

  return GAME_DRAW;
}

void pc_turn() {
  int row;
  int col;

  do {
    row = random_range(0, 2);
    col = random_range(0, 2);
  } while (board[row][col] != NONE);

  board[row][col] = PLAYER_PC;
}

static void publish_score_task(void *args) {
  earned_coins = game_publish_score("tictactoe", calculate_score());
  wallet_refresh_async();
  spinner_stop();
  overlay_show(&game_over_overlay);
}

static Option game_over_options[2] = {
    {.key = "Revancha", .value = 0},
    {.key = "Salir", .value = 1},
};

static Menu game_over_menu = {
    .buzzer = &buzzer,
    .length = 2,
    .current = 0,
    .selected = -1,
    .on_select = game_over_menu_select,
    .options = game_over_options,
};

static void game_over_init(GameResult result) {
  game_state = TTT_GAME_OVER;
  final_result = result;
  game_over_menu.current = 0;
  game_over_menu.selected = -1;

  earned_coins = 0;

  spinner_start(oled.width / 2, oled.height / 2);
  task_start("ttt_score", publish_score_task, NULL);
}

static void game_over_render(Overlay *self) {
  Modal modal = {
      .menu = &game_over_menu,
      .height = 60,
      .width = 80,
      .x = 24,
      .y = 2,
  };

  switch (final_result) {
  case GAME_ONGOING:
    return;

  case GAME_PLAYER_WON:
    snprintf(modal.title, sizeof(modal.title), "%s", "Ganaste!");
    snprintf(modal.description, sizeof(modal.description), "%d puntos",
             calculate_score());
    break;

  case GAME_PC_WON:
    snprintf(modal.title, sizeof(modal.title), "%s", "Perdiste :(");
    break;

  case GAME_DRAW:
    snprintf(modal.title, sizeof(modal.title), "%s", "Empate...");
    break;
  }

  if (final_result == GAME_PLAYER_WON) {
    ModalItem items[] = {{
        .bitmap = icons_rpg_coin_gold_currency_money_gp(),
        .bitmap_width = ICON_WIDTH,
        .bitmap_height = ICON_HEIGHT,
        .value = earned_coins,
    }};

    modal.item_count = 1;
    modal.items = items;
  }
  oled_draw_modal(&oled, &modal);
}

static void game_over_update(Overlay *self) {
  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (click == ROTARY_CLICK_SINGLE) {
    rotary_encoder_wait_for_release(&rotary_encoder);
    menu_select(&game_over_menu);
  }

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&game_over_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&game_over_menu);
}

static void game_over_menu_select(void *ctx, int32_t selected) {
  overlay_remove(&game_over_overlay);
  if (selected == 0)
    restart();
  else if (selected == 1)
    screen_pop();
}

static void move_cursor(RotaryDirection direction) {
  if (direction == ROTARY_CLOCKWISE)
    buzzer_set_frequency(&buzzer, 393);
  else
    buzzer_set_frequency(&buzzer, 550);

  buzzer_tone_ms(buzzer, 100);

  int index = cursor_row * 3 + cursor_col;

  if (direction == ROTARY_CLOCKWISE)
    index = (index + 1) % 9;
  else
    index = (index + 8) % 9;

  cursor_row = index / 3;
  cursor_col = index % 3;
}

static void player_turn(void) {
  if (board[cursor_row][cursor_col] != NONE)
    return;

  board[cursor_row][cursor_col] = PLAYER_1;
}

static void ttt_update(Screen *self) {
  if (game_state != TTT_PLAYING)
    return;

  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (direction == ROTARY_CLOCKWISE || direction == ROTARY_COUNTERCLOCKWISE) {
    move_cursor(direction);
  }

  if (click == ROTARY_CLICK_SINGLE) {
    rotary_encoder_wait_for_release(&rotary_encoder);
    if (board[cursor_row][cursor_col] != NONE)
      return;
    player_turn();
    GameResult result = check_end();
    if (result != GAME_ONGOING)
      return game_over_init(result);
    pc_turn();
    result = check_end();
    if (result != GAME_ONGOING)
      return game_over_init(result);
  }
}

static void ttt_render(Screen *self) {
  draw_board();

  int x = col_to_x(cursor_col) + 2;
  int y = row_to_y(cursor_row) + 2;
  oled_fill_square(&oled, x, y, CELL_SIZE - 4, true);
  if (board[cursor_row][cursor_col] != NONE)
    draw_cell(cursor_row, cursor_col, false);
}

static Screen ttt_screen = {
    .render = ttt_render,
    .update = ttt_update,
};

void tictactoe_play(void) {
  restart();
  screen_push(ttt_screen);
}
