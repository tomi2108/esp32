#include "snake.h"
#include "buzzer.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>

#define CELL_SIZE 4
#define ROWS (64 / CELL_SIZE)
#define COLS (128 / CELL_SIZE)
#define MOVE_INTERVAL_MS 200
#define INPUT_INTERVAL_MS 10
#define MAX_SNAKE_LENGTH (ROWS * COLS)

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

typedef struct {
  int row;
  int col;
} Cell;

typedef enum CellValue { NONE, APPLE, SNAKE } CellValue;

typedef enum Direction {
  UP = 0,
  RIGHT = 1,
  DOWN = 2,
  LEFT = 3,
} Direction;

static Cell snake_body[MAX_SNAKE_LENGTH];
static int snake_length;
static int head_index;

static CellValue board[ROWS][COLS];
static Direction current_direction;

static const Direction directions[4] = {UP, RIGHT, DOWN, LEFT};

static int get_tail_index(void) {
  return (head_index - snake_length + MAX_SNAKE_LENGTH) % MAX_SNAKE_LENGTH;
}

static Cell get_tail(void) { return snake_body[get_tail_index()]; }

static Cell get_head(void) { return snake_body[head_index]; }

static Direction get_next_direction(Direction curr) {
  return directions[(curr + 1) % 4];
}

static Direction get_prev_direction(Direction curr) {
  return directions[(curr + 3) % 4];
}

static Direction get_opposite_direction(Direction direction) {
  return (direction + 2) % 4;
}

static void generate_new_apple(void) {
  int row;
  int col;

  do {
    row = random_range(0, ROWS - 1);
    col = random_range(0, COLS - 1);
  } while (board[row][col] != NONE);

  board[row][col] = APPLE;
}

static void handle_input(void *args) {
  while (1) {
    RotaryDirection dir = rotary_encoder_direction(&rotary_encoder);
    Direction next_direction = current_direction;

    if (dir == ROTARY_CLOCKWISE)
      next_direction = get_next_direction(current_direction);

    if (dir == ROTARY_COUNTERCLOCKWISE)
      next_direction = get_prev_direction(current_direction);

    if (next_direction != get_opposite_direction(current_direction))
      current_direction = next_direction;

    vTaskDelay(pdMS_TO_TICKS(INPUT_INTERVAL_MS));
  }
}

static void restart(void) {
  task_start("snake_input", handle_input, NULL);
  memset(board, NONE, sizeof(board));
  snake_length = 2;
  head_index = 1;
  current_direction = LEFT;
  snake_body[0] = (Cell){
      .row = ROWS / 2,
      .col = COLS / 2 + 1,
  };
  snake_body[1] = (Cell){
      .row = ROWS / 2,
      .col = COLS / 2,
  };
  for (int i = 0; i < snake_length; i++) {
    Cell cell = snake_body[i];
    board[cell.row][cell.col] = SNAKE;
  }
  generate_new_apple();
}

static void draw_cell(int row, int col) {
  int x = col * CELL_SIZE;
  int y = row * CELL_SIZE;

  switch (board[row][col]) {
  case NONE:
    break;
  case SNAKE:
    oled_fill_square(&oled, x, y, CELL_SIZE, true);
    break;
  case APPLE:
    oled_draw_circle(&oled, x, y, 0, CELL_SIZE / 2, true);
    break;
  }
}

static void board_display(void) {
  for (int row = 0; row < ROWS; row++)
    for (int col = 0; col < COLS; col++)
      draw_cell(row, col);

  oled_update(&oled);
}

static void input_stop(void) { task_stop("snake_input"); }

static bool move(void) {
  Cell new_head = get_head();

  switch (current_direction) {
  case LEFT:
    new_head.col--;
    break;
  case UP:
    new_head.row--;
    break;
  case DOWN:
    new_head.row++;
    break;
  case RIGHT:
    new_head.col++;
    break;
  }

  if (new_head.col < 0 || new_head.col >= COLS || new_head.row < 0 ||
      new_head.row >= ROWS)
    return true;

  bool eating = board[new_head.row][new_head.col] == APPLE;
  Cell tail = get_tail();

  bool moving_into_tail = new_head.row == tail.row && new_head.col == tail.col;

  if (board[new_head.row][new_head.col] == SNAKE &&
      (eating || !moving_into_tail))
    return true;

  if (!eating) {
    board[tail.row][tail.col] = NONE;
  } else {
    snake_length++;
  }

  head_index = (head_index + 1) % MAX_SNAKE_LENGTH;
  snake_body[head_index] = new_head;

  board[new_head.row][new_head.col] = SNAKE;

  if (eating) {
    buzzer_set_frequency(&buzzer, 440);
    buzzer_tone_ms(buzzer, 100);
    generate_new_apple();
  }

  return false;
}

static int calculate_score(void) {
  int apples_eaten = snake_length - 2;
  int score = apples_eaten * 50;
  return score > 1000 ? 1000 : score;
}

bool show_retry(void) {
  Option options[2] = {
      {.key = "Reintentar", .value = 0},
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

  if (snake_length == MAX_SNAKE_LENGTH)
    snprintf(modal.title, sizeof(modal.title), "%s", "Ganaste!");
  else
    snprintf(modal.title, sizeof(modal.title), "%s", "Perdiste :(");

  int score = calculate_score();
  snprintf(modal.description, 12, "%d puntos", score);
  spinner_start(&oled, 65, 32);
  int32_t coins = api_publish_score("snake", score);
  spinner_stop();

  ModalItem items[] = {{
      .bitmap = icons_rpg_coin_gold_currency_money_gp(),
      .bitmap_width = ICON_WIDTH,
      .bitmap_height = ICON_HEIGHT,
      .value = coins,
  }};
  modal.item_count = 1;
  modal.items = items;

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

void snake_play(void) {
  restart();
  while (1) {
    bool game_over = move();
    if (game_over) {
      input_stop();
      if (show_retry())
        restart();
      else
        break;
    }
    oled_clear(&oled);
    board_display();
    vTaskDelay(pdMS_TO_TICKS(MOVE_INTERVAL_MS));
  }
}
