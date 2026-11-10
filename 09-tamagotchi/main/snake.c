#include "snake.h"

#define CELL_SIZE 4
#define ROWS 64 / CELL_SIZE
#define COLS 128 / CELL_SIZE

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

typedef struct {
  int row;
  int col;
} Cell;
typedef enum Snake_CellValue { NONE, APPLE, SNAKE } Snake_CellValue;
typedef enum Direction {
  UP = 0,
  RIGHT = 1,
  DOWN = 2,
  LEFT = 3,
} Direction;

static TaskHandle_t input_task_handle;
int snake_length = 2;
Cell head = {.col = COLS / 2, .row = ROWS / 2};
Snake_CellValue snake_board[ROWS][COLS] = {0};
Direction current_direction;

Direction directions[4] = {UP, RIGHT, DOWN, LEFT};
Direction get_next_direction(Direction curr) {
  return directions[(curr + 1) % 4];
}

Direction get_prev_direction(Direction curr) {
  return directions[(curr - 1) % 4];
}

void generate_new_apple() {
  int row;
  int col;
  bool free = false;
  while (!free) {
    row = random_range(0, ROWS - 1);
    col = random_range(0, COLS - 1);
    if (snake_board[row][col] == NONE)
      free = true;
  }
  snake_board[row][col] = APPLE;
}

void snake_restart() {
  for (int row = 0; row < ROWS; row++)
    for (int col = 0; col < COLS; col++)
      snake_board[row][col] = NONE;

  head.row = ROWS / 2;
  head.col = COLS / 2;
  current_direction = LEFT;
  snake_board[head.row][head.col] = SNAKE;
  snake_board[head.row][head.col + 1] = SNAKE;
  generate_new_apple();
}

void handle_input(void *args) {
  while (1) {
    RotaryDirection dir = rotary_encoder_direction(&rotary_encoder);
    if (dir == ROTARY_CLOCKWISE)
      current_direction = get_next_direction(current_direction);
    if (dir == ROTARY_COUNTERCLOCKWISE)
      current_direction = get_prev_direction(current_direction);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void snake_draw_cell(int row, int col) {
  int x = col * CELL_SIZE;
  int y = row * CELL_SIZE;

  switch (snake_board[row][col]) {
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

void snake_board_display() {
  for (int row = 0; row < ROWS; row++)
    for (int col = 0; col < COLS; col++)
      snake_draw_cell(row, col);
  oled_update(&oled);
}

Cell get_tail(void) {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      if (snake_board[row][col] != SNAKE ||
          (head.row == row && head.col == col))
        continue;

      int free_cells = 0;
      if (row + 1 >= ROWS || snake_board[row + 1][col] != SNAKE)
        free_cells++;
      if (row - 1 < 0 || snake_board[row - 1][col] != SNAKE)
        free_cells++;
      if (col + 1 >= COLS || snake_board[row][col + 1] != SNAKE)
        free_cells++;
      if (col - 1 < 0 || snake_board[row][col - 1] != SNAKE)
        free_cells++;
      if (free_cells == 3)
        return (Cell){.row = row, .col = col};
    }
  }
  return head;
}

void input_stop() {
  vTaskDelete(input_task_handle);
  input_task_handle = NULL;
}

bool move() {
  Cell new_head = head;
  Cell tail = get_tail();
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
      new_head.row >= ROWS) {
    return true;
  }

  switch (snake_board[new_head.row][new_head.col]) {
  case NONE:
    snake_board[new_head.row][new_head.col] = SNAKE;
    snake_board[tail.row][tail.col] = NONE;
    break;
  case APPLE:
    snake_board[new_head.row][new_head.col] = SNAKE;
    snake_length++;
    generate_new_apple();
    break;
  case SNAKE:
    input_stop();
    return true;
  }
  head = new_head;
  return false;
}

void snake_play(void) {
  bool back = false;
  snake_restart();
  xTaskCreate(handle_input, "input_task", 4096, NULL, 2, &input_task_handle);
  while (!back) {
    oled_clear(&oled);
    snake_board_display();
    move();
    vTaskDelay(pdMS_TO_TICKS(700));
  }
}
