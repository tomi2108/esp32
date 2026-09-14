#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "lcd.h"

#define BUTTON_GPIO GPIO_NUM_4
#define BUZZER_GPIO GPIO_NUM_16
#define LEDS 4

#define LCD_SDA GPIO_NUM_21
#define LCD_SCL GPIO_NUM_22

#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988

typedef enum Duration {
  WHOLE = 1,
  HALF = 2,
  QUARTER = 4,
  EIGHTH = 8,
  D_WHOLE = -1,
  D_HALF = -2,
  D_QUARTER = -4,
  D_EIGHTH = -8,
} Duration;

typedef struct Note {
  int note;
  Duration duration;
} Note;

static i2c_master_bus_handle_t i2c_bus;
static i2c_master_dev_handle_t lcd_dev;

int ledPins[LEDS] = {
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_25,
    GPIO_NUM_26,
};

void button_init(void) { gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT); }
int button_is_pressed(void) { return gpio_get_level(BUTTON_GPIO) == 1; }

void leds_init() {
  for (int i = 0; i < LEDS; i++)
    gpio_set_direction(ledPins[i], GPIO_MODE_OUTPUT);
}

void leds_set(int index) {
  for (int i = 0; i < LEDS; i++)
    gpio_set_level(ledPins[i], i == index);
}

void leds_stop() {
  for (int i = 0; i < LEDS; i++)
    gpio_set_level(ledPins[i], 0);
}

void buzzer_init(void) {
  gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
  ledc_timer_config_t timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .timer_num = LEDC_TIMER_0,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .freq_hz = 440,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  ledc_channel_config_t channel = {
      .gpio_num = BUZZER_GPIO,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0,
  };

  ledc_timer_config(&timer);
  ledc_channel_config(&channel);
}

void buzzer_tone(int frequency) {
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequency);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_stop(void) {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

int default_message;

void lcd_default_message() {
  if (default_message)
    return;
  default_message = 1;
  lcd_clear(lcd_dev);
  lcd_set_cursor(lcd_dev, 0, 0);
  lcd_print(lcd_dev, "No tocar");
  lcd_set_cursor(lcd_dev, 1, 0);
  lcd_print(lcd_dev, "Excepto Rochi :)");
}

void lcd_on_message() {
  if (!default_message)
    return;
  default_message = 0;
  lcd_clear(lcd_dev);
  lcd_set_cursor(lcd_dev, 0, 0);
  lcd_print(lcd_dev, "Feliz cumplee!!!");
  lcd_set_cursor(lcd_dev, 1, 0);
  lcd_print(lcd_dev, "Te amo muchoo <3");
}

void app_main(void) {
  leds_init();
  buzzer_init();
  lcd_init(&i2c_bus, &lcd_dev, LCD_SDA, LCD_SCL);
  lcd_default_message();

  int tempo = 140;
  Note melody[] = {
      {NOTE_C4, QUARTER},   {NOTE_C4, EIGHTH},    {NOTE_D4, D_QUARTER},
      {NOTE_C4, D_QUARTER}, {NOTE_F4, D_QUARTER}, {NOTE_E4, D_HALF},
      {NOTE_C4, QUARTER},   {NOTE_C4, EIGHTH},    {NOTE_D4, D_QUARTER},
      {NOTE_C4, D_QUARTER}, {NOTE_G4, D_QUARTER}, {NOTE_F4, D_HALF},
      {NOTE_C4, QUARTER},   {NOTE_C4, EIGHTH},    {NOTE_C5, D_QUARTER},
      {NOTE_A4, D_QUARTER}, {NOTE_F4, D_QUARTER}, {NOTE_E4, D_QUARTER},
      {NOTE_D4, D_QUARTER}, {NOTE_AS4, QUARTER},  {NOTE_AS4, EIGHTH},
      {NOTE_A4, D_QUARTER}, {NOTE_F4, D_QUARTER}, {NOTE_G4, D_QUARTER},
      {NOTE_F4, D_HALF},
  };

  while (1) {
    if (button_is_pressed()) {
      lcd_on_message();
      int notes = sizeof(melody) / sizeof(Note);
      int wholenote = (60000 * 4) / tempo;
      int noteDuration = 0;

      for (int i = 0; i < notes; i++) {
        Note note = melody[i];

        if (note.duration > 0)
          noteDuration = (wholenote) / note.duration;
        else if (note.duration < 0)
          noteDuration = (wholenote) * 1.5 / abs(note.duration);

        leds_set(i % LEDS);
        buzzer_tone(note.note);
        vTaskDelay(pdMS_TO_TICKS(noteDuration * 0.9));
        buzzer_stop();
        vTaskDelay(pdMS_TO_TICKS(noteDuration * 0.1));
      }

    } else {
      lcd_default_message();
      buzzer_stop();
      leds_stop();
    }
  }
}
