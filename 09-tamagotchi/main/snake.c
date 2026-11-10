#include "snake.h"

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

typedef enum Snake_CellValue { NONE, APPLE, SNAKE } Snake_CellValue;

typedef enum Direction {
  UP = 0,
  RIGHT = 1,
  DOWN = 2,
  LEFT = 3,
} Direction;

static TaskHandle_t input_task_handle;

static Cell snake_body[MAX_SNAKE_LENGTH];
static int snake_length;
static int head_index;

static Snake_CellValue snake_board[ROWS][COLS];
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
  } while (snake_board[row][col] != NONE);

  snake_board[row][col] = APPLE;
}

static void snake_restart(void) {
  memset(snake_board, NONE, sizeof(snake_board));
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
    snake_board[cell.row][cell.col] = SNAKE;
  }
  generate_new_apple();
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

static void snake_draw_cell(int row, int col) {
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

static void snake_board_display(void) {
  for (int row = 0; row < ROWS; row++)
    for (int col = 0; col < COLS; col++)
      snake_draw_cell(row, col);

  oled_update(&oled);
}

static void input_stop(void) {
  if (input_task_handle != NULL) {
    vTaskDelete(input_task_handle);
    input_task_handle = NULL;
  }
}

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

  bool eating = snake_board[new_head.row][new_head.col] == APPLE;
  Cell tail = get_tail();

  bool moving_into_tail = new_head.row == tail.row && new_head.col == tail.col;

  if (snake_board[new_head.row][new_head.col] == SNAKE &&
      (eating || !moving_into_tail))
    return true;

  if (!eating) {
    snake_board[tail.row][tail.col] = NONE;
  } else {
    snake_length++;
  }

  head_index = (head_index + 1) % MAX_SNAKE_LENGTH;
  snake_body[head_index] = new_head;

  snake_board[new_head.row][new_head.col] = SNAKE;

  if (eating)
    generate_new_apple();

  return false;
}

void snake_play(void) {
  snake_restart();

  xTaskCreate(handle_input, "input_task", 4096, NULL, 2, &input_task_handle);

  bool game_over = false;

  while (!game_over) {
    game_over = move();

    oled_clear(&oled);
    snake_board_display();

    vTaskDelay(pdMS_TO_TICKS(MOVE_INTERVAL_MS));
  }

  input_stop();
}
