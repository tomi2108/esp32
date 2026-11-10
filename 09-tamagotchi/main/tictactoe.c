#include "tictactoe.h"

#define CELL_SIZE 20
#define PADDING_X 34
#define PADDING_Y 2

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

typedef enum TTT_CellValue { NONE, PLAYER_1, PLAYER_PC } TTT_CellValue;
typedef enum {
  GAME_ONGOING,
  GAME_PLAYER_WON,
  GAME_PC_WON,
  GAME_DRAW
} TTT_GameResult;

TTT_CellValue ttt_board[3][3] = {0};

void draw_player_one(int row, int col, bool on) {
  int x = PADDING_X + col * CELL_SIZE;
  int y = PADDING_Y + row * CELL_SIZE;
  int padding = 5;
  oled_draw_line(&oled, x + padding, y + padding, x + CELL_SIZE - padding,
                 y + CELL_SIZE - padding, on);
  oled_draw_line(&oled, x + CELL_SIZE - padding, y + padding, x + padding,
                 y + CELL_SIZE - padding, on);
}

void draw_player_two(int row, int col, bool on) {
  int x = PADDING_X + col * CELL_SIZE;
  int y = PADDING_Y + row * CELL_SIZE;
  oled_draw_circle(&oled, x + CELL_SIZE / 2, y + CELL_SIZE / 2, 5, 7, on);
}

void ttt_draw_cell(int row, int col, bool on) {
  switch (ttt_board[row][col]) {
  case NONE:
    break;
  case PLAYER_1:
    draw_player_one(row, col, on);
    break;

  case PLAYER_PC:
    draw_player_two(row, col, on);
    break;
  }
}

void ttt_board_display(void) {
  for (int i = 1; i < 3; i++) {
    int y = PADDING_Y + CELL_SIZE * i;
    oled_draw_line(&oled, PADDING_X, y, PADDING_X + CELL_SIZE * 3, y, true);
  }

  for (int i = 1; i < 3; i++) {
    int x = PADDING_X + CELL_SIZE * i;
    oled_draw_line(&oled, x, PADDING_Y, x, PADDING_Y + CELL_SIZE * 3, true);
  }

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      ttt_draw_cell(row, col, true);
    }
  }

  oled_update(&oled);
}

void play_turn() {
  int row = 0;
  int col = 0;
  bool selected = false;

  while (!selected) {
    int x = PADDING_X + 1 + col * CELL_SIZE;
    int y = PADDING_Y + 1 + row * CELL_SIZE;

    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_SINGLE) {
      rotary_encoder_wait_for_release(&rotary_encoder);
      if (ttt_board[row][col] == NONE)
        selected = true;
    }

    if (direction == ROTARY_CLOCKWISE) {
      buzzer_set_frequency(&buzzer, 393);
      buzzer_tone_ms(buzzer, 100);
      oled_fill_square(&oled, x, y, CELL_SIZE - 2, false);
      ttt_draw_cell(row, col, true);
      col++;
    }

    if (direction == ROTARY_COUNTERCLOCKWISE) {
      buzzer_set_frequency(&buzzer, 550);
      buzzer_tone_ms(buzzer, 100);
      oled_fill_square(&oled, x, y, CELL_SIZE - 2, false);
      ttt_draw_cell(row, col, true);
      col--;
    }

    if (col == 3) {
      col = 0;
      row++;
    }
    if (col == -1) {
      col = 2;
      row--;
    }
    if (row == 3) {
      col = 0;
      row = 0;
    }
    if (row == -1) {
      col = 2;
      row = 2;
    }

    x = PADDING_X + 1 + col * CELL_SIZE;
    y = PADDING_Y + 1 + row * CELL_SIZE;
    oled_fill_square(&oled, x, y, CELL_SIZE - 2, true);
    ttt_draw_cell(row, col, false);

    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  ttt_board[row][col] = PLAYER_1;
}

void pc_turn() {
  int row;
  int col;
  bool taken = true;
  while (taken) {
    row = random_range(0, 2);
    col = random_range(0, 2);
    if (ttt_board[row][col] == NONE)
      taken = false;
  }
  ttt_board[row][col] = PLAYER_PC;
}

int calculate_score(TTT_GameResult result) {
  if (result != GAME_PLAYER_WON)
    return 0;
  int pieces = 0;
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (ttt_board[row][col] != NONE)
        pieces++;
    }
  }
  int score = 1000 - ((pieces - 5) * 200);
  if (score < 0)
    score = 0;
  if (score > 1000)
    score = 1000;
  return score;
}

bool show_retry(TTT_GameResult result) {
  Option options[2] = {
      {.key = "Revancha", .value = 0},
      {.key = "Salir", .value = 1},
  };
  Menu menu = {
      .buzzer = &buzzer,
      .length = 2,
      .current = 0,
      .selected = -1,
      .options = options,
  };
  Modal modal = {
      .menu = &menu,
      .height = 60,
      .width = 80,
      .x = 24,
      .y = 2,
  };

  switch (result) {
  case GAME_ONGOING:
    return false;
  case GAME_PLAYER_WON:
    modal.title = "Ganaste!";
    break;
  case GAME_PC_WON:
    modal.title = "Perdiste!";
    break;
  case GAME_DRAW:
    modal.title = "Empate!";
    break;
  }

  if (result == GAME_PLAYER_WON) {
    int score = calculate_score(result);
    spinner_start(&oled, 64, 32);
    Response *res = api_publish_score("tictactoe", score);
    spinner_stop();

    t_json_value *res_body = json_parse_buffer(res->body);
    int32_t coins_won;
    json_value_int(json_object_get(res_body, "coins"), &coins_won);

    ModalItem items[] = {{
        .bitmap = icons_rpg_coin_gold_currency_money_gp(),
        .bitmap_width = ICON_WIDTH,
        .bitmap_height = ICON_HEIGHT,
        .value = coins_won,
    }};
    modal.item_count = 1;
    modal.items = items;
  }

  while (menu.selected == -1) {
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_SINGLE) {
      rotary_encoder_wait_for_release(&rotary_encoder);
      menu_select(&menu);
    }

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&menu);

    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&menu);

    oled_clear(&oled);
    oled_draw_modal(&oled, &modal);
    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  return menu.selected == 0;
}

void ttt_restart() {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      ttt_board[i][j] = NONE;
}

TTT_GameResult check_end(void) {
  static const int wins[8][3][2] = {
      {{0, 0}, {0, 1}, {0, 2}}, {{1, 0}, {1, 1}, {1, 2}},
      {{2, 0}, {2, 1}, {2, 2}}, {{0, 0}, {1, 0}, {2, 0}},
      {{0, 1}, {1, 1}, {2, 1}}, {{0, 2}, {1, 2}, {2, 2}},
      {{0, 0}, {1, 1}, {2, 2}}, {{0, 2}, {1, 1}, {2, 0}},
  };

  for (int i = 0; i < 8; i++) {
    TTT_CellValue value = ttt_board[wins[i][0][0]][wins[i][0][1]];
    if (value != NONE && value == ttt_board[wins[i][1][0]][wins[i][1][1]] &&
        value == ttt_board[wins[i][2][0]][wins[i][2][1]])
      return value == PLAYER_1 ? GAME_PLAYER_WON : GAME_PC_WON;
  }

  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      if (ttt_board[row][col] == NONE)
        return GAME_ONGOING;
  return GAME_DRAW;
}

void tictactoe_play(void) {
  bool back = false;
  while (!back) {
    oled_clear(&oled);
    ttt_board_display();
    play_turn();
    TTT_GameResult result = check_end();
    if (result == GAME_ONGOING) {
      pc_turn();
      result = check_end();
    }
    ttt_board_display();
    if (result != GAME_ONGOING) {
      if (show_retry(result))
        ttt_restart();
      else
        back = true;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
