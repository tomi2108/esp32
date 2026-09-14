#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#define BUTTON_GPIO GPIO_NUM_4
#define BUZZER_GPIO GPIO_NUM_16
#define LEDS 4

#define LCD_SDA GPIO_NUM_21
#define LCD_SCL GPIO_NUM_22
#define LCD_ADDR 0x27
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_REG_SELECT 0x01

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

void lcd_write_byte(uint8_t data) {
  i2c_master_transmit(lcd_dev, &data, 1, -1);
}

void lcd_pulse_enable(uint8_t data) {
  lcd_write_byte(data | LCD_ENABLE);
  esp_rom_delay_us(1);
  lcd_write_byte(data & ~LCD_ENABLE);
  esp_rom_delay_us(50);
}

void lcd_write_nibble(uint8_t nibble, uint8_t control) {
  uint8_t data = (nibble & 0xF0) | control | LCD_BACKLIGHT;
  lcd_pulse_enable(data);
}

void lcd_send(uint8_t value, uint8_t control) {
  lcd_write_nibble(value & 0xF0, control);
  lcd_write_nibble((value << 4) & 0xF0, control);
}

void lcd_command(uint8_t command) { lcd_send(command, 0); }
void lcd_data(uint8_t data) { lcd_send(data, LCD_REG_SELECT); }
void lcd_clear(void) {
  lcd_command(0x01);
  vTaskDelay(pdMS_TO_TICKS(2));
}
void lcd_set_cursor(uint8_t row, uint8_t column) {
  uint8_t address;
  if (row == 0)
    address = 0x00 + column;
  else if (row == 1)
    address = 0x40 + column;
  else
    return;

  lcd_command(0x80 | address);
}
void lcd_print(const char *text) {
  while (*text)
    lcd_data((uint8_t)*text++);
}

void lcd_init(void) {
  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = LCD_SDA,
      .scl_io_num = LCD_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  i2c_device_config_t dev_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = LCD_ADDR,
      .scl_speed_hz = 100000,
  };
  i2c_new_master_bus(&bus_config, &i2c_bus);
  i2c_master_bus_add_device(i2c_bus, &dev_config, &lcd_dev);

  vTaskDelay(pdMS_TO_TICKS(50));
  lcd_write_nibble(0x30, 0);
  esp_rom_delay_us(150);
  lcd_write_nibble(0x30, 0);
  esp_rom_delay_us(150);
  lcd_write_nibble(0x30, 0);
  esp_rom_delay_us(150);

  lcd_write_nibble(0x20, 0);

  lcd_command(0x28);
  lcd_command(0x0C);
  lcd_command(0x06);
  lcd_clear();
}

int default_message;

void lcd_default_message() {
  if (default_message)
    return;
  default_message = 1;
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_print("No tocar");
  lcd_set_cursor(1, 0);
  lcd_print("Excepto Rochi :)");
}

void lcd_on_message() {
  if (!default_message)
    return;
  default_message = 0;
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_print("Feliz cumplee!!!");
  lcd_set_cursor(1, 0);
  lcd_print("Te amo muchoo <3");
}

void app_main(void) {
  leds_init();
  buzzer_init();
  lcd_init();
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
