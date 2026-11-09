#include "tictactoe.h"
#include "icons.h"
#include "oled.h"

#define CELL_SIZE 20
#define PADDING_X 34
#define PADDING_Y 2

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

typedef enum CellValue { NONE, PLAYER_1, PLAYER_2 } CellValue;

CellValue board[3][3] = {{NONE, PLAYER_1, PLAYER_2},
                         {NONE, NONE, PLAYER_2},
                         {PLAYER_1, NONE, PLAYER_2}};

void board_display(void) {
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
      int x = PADDING_X + col * CELL_SIZE;
      int y = PADDING_Y + row * CELL_SIZE;

      switch (board[row][col]) {
      case NONE:
        break;

      case PLAYER_1:
        oled_draw_circle(&oled, x + CELL_SIZE / 2, y + CELL_SIZE / 2, 5, 7,
                         true);
        break;

      case PLAYER_2:
        oled_draw_bitmap(
            &oled, x + 2, y + 2, ICON_HEIGHT,
            ICON_WIDTH, false,
            icons_software_signs_maths_multiplication_x_crossout_checkmark_cancel());
        break;
      }
    }
  }

  oled_update(&oled);
}

void tictactoe_play(void) {
  oled_clear(&oled);

  bool back = false;

  while (!back) {
    board_display();
  }
}
