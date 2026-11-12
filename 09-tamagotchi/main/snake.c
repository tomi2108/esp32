#include "snake.h"
#include "overlay.h"
#include "screen.h"
#include "spinner.h"
#include "task.h"
#include "wallet.h"

#define CELL_SIZE 4
#define ROWS (64 / CELL_SIZE)
#define COLS (128 / CELL_SIZE)
#define MOVE_INTERVAL_MS 200
#define MAX_SNAKE_LENGTH (ROWS * COLS)

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

typedef enum {
  SNAKE_PLAYING,
  SNAKE_GAME_OVER,
} SnakeState;

typedef struct {
  int row;
  int col;
} Cell;

typedef enum { NONE, APPLE, SNAKE } CellValue;

typedef enum {
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
static SnakeState snake_state;

static TickType_t last_move_tick;

static volatile bool score_loading;
static volatile bool score_loaded;
static volatile int32_t earned_coins;

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

static void restart(void) {
  snake_state = SNAKE_PLAYING;
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
  last_move_tick = xTaskGetTickCount();
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

  if (!eating)
    board[tail.row][tail.col] = NONE;
  else
    snake_length++;

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

static void publish_score_task(void *args) {
  earned_coins = api_publish_score("snake", calculate_score());
  wallet_refresh_async();
  score_loading = false;
  score_loaded = true;
}

static Option game_over_options[2] = {
    {.key = "Reintentar", .value = 0},
    {.key = "Salir", .value = 1},
};

static void game_over_menu_select(void *ctx, int32_t selected);

static Menu game_over_menu = {
    .buzzer = &buzzer,
    .length = 2,
    .current = 0,
    .selected = -1,
    .on_select = game_over_menu_select,
    .options = game_over_options,
};

static void game_over_init(void) {
  snake_state = SNAKE_GAME_OVER;
  game_over_menu.current = 0;
  game_over_menu.selected = -1;
  earned_coins = 0;
  score_loading = true;
  score_loaded = false;
  spinner_start(&oled, 64, 32);
  if (task_start("snake_score", publish_score_task, NULL) != 0) {
    score_loading = false;
    score_loaded = true;
  }
}

static void game_over_render(Overlay *self) {
  if (score_loading || score_loaded)
    return;

  Modal modal = {
      .menu = &game_over_menu,
      .height = 60,
      .width = 80,
      .x = 24,
      .y = 2,
  };

  if (snake_length == MAX_SNAKE_LENGTH)
    snprintf(modal.title, sizeof(modal.title), "%s", "Ganaste!");
  else
    snprintf(modal.title, sizeof(modal.title), "%s", "Perdiste :(");

  snprintf(modal.description, sizeof(modal.description), "%d puntos",
           calculate_score());

  ModalItem items[] = {{
      .bitmap = icons_rpg_coin_gold_currency_money_gp(),
      .bitmap_width = ICON_WIDTH,
      .bitmap_height = ICON_HEIGHT,
      .value = earned_coins,
  }};

  modal.item_count = 1;
  modal.items = items;

  oled_draw_modal(&oled, &modal);
}

static void game_over_update(Overlay *self) {
  if (score_loading)
    return;

  if (score_loaded) {
    spinner_stop();
    score_loaded = false;
    return;
  }

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

static Overlay game_over_overlay = {
    .render = game_over_render,
    .update = game_over_update,
};

static void game_over_menu_select(void *ctx, int32_t selected) {
  overlay_remove(&game_over_overlay);
  if (selected == 0)
    restart();
  else if (selected == 1)
    screen_pop();
}

static void snake_update(Screen *self) {
  if (snake_state != SNAKE_PLAYING)
    return;

  RotaryDirection dir = rotary_encoder_direction(&rotary_encoder);

  Direction next = current_direction;

  if (dir == ROTARY_CLOCKWISE)
    next = get_next_direction(current_direction);

  if (dir == ROTARY_COUNTERCLOCKWISE)
    next = get_prev_direction(current_direction);

  if (next != get_opposite_direction(current_direction))
    current_direction = next;

  TickType_t now = xTaskGetTickCount();

  if (now - last_move_tick >= pdMS_TO_TICKS(MOVE_INTERVAL_MS)) {
    last_move_tick = now;

    if (move()) {
      game_over_init();
      overlay_show(&game_over_overlay);
    }
  }
}

static void snake_render(Screen *self) {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      draw_cell(row, col);
    }
  }
}

static Screen snake_screen = {
    .render = snake_render,
    .update = snake_update,
};

void snake_play(void) {
  restart();
  screen_push(snake_screen);
}
