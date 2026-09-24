#include "lcd.h"

static void lcd_write_byte(LCD lcd, uint8_t data) {
  i2c_write(lcd.device, &data, 1);
}

static void lcd_pulse_enable(LCD lcd, uint8_t data) {
  lcd_write_byte(lcd, data | LCD_ENABLE);
  esp_rom_delay_us(1);
  lcd_write_byte(lcd, data & ~LCD_ENABLE);
  esp_rom_delay_us(50);
}

static void lcd_write_nibble(LCD lcd, uint8_t nibble, uint8_t control) {
  uint8_t data = (nibble & 0xF0) | control | LCD_BACKLIGHT;
  lcd_pulse_enable(lcd, data);
}

static void lcd_send(LCD lcd, uint8_t value, uint8_t control) {
  lcd_write_nibble(lcd, value & 0xF0, control);
  lcd_write_nibble(lcd, (value << 4) & 0xF0, control);
}

static void lcd_command(LCD lcd, uint8_t command) { lcd_send(lcd, command, 0); }

static void lcd_data(LCD lcd, uint8_t data) {
  lcd_send(lcd, data, LCD_REG_SELECT);
}

LCD lcd_init(uint8_t rows, uint8_t columns, gpio_num_t sda, gpio_num_t scl) {
  i2c_master_bus_handle_t bus = i2c_bus_init(sda, scl);
  i2c_master_dev_handle_t device = i2c_device_init(bus, LCD_ADDR, 100000);
  LCD lcd = {
      .bus = bus,
      .device = device,
      .rows = rows,
      .columns = columns,
  };

  vTaskDelay(pdMS_TO_TICKS(50));

  // Force 8-bit mode.
  lcd_write_nibble(lcd, 0x30, 0);
  vTaskDelay(pdMS_TO_TICKS(5));
  lcd_write_nibble(lcd, 0x30, 0);
  esp_rom_delay_us(150);
  lcd_write_nibble(lcd, 0x30, 0);

  esp_rom_delay_us(150);

  // Switch to 4-bit mode.
  lcd_write_nibble(lcd, 0x20, 0);
  vTaskDelay(pdMS_TO_TICKS(1));

  // Function set:
  // 4-bit, 2 lines, 5x8 font.
  lcd_command(lcd, 0x28);

  // Display OFF.
  lcd_command(lcd, 0x08);
  // Clear display.
  lcd_clear(lcd);
  // Entry mode:
  // Increment cursor, no shift.
  lcd_command(lcd, 0x06);
  // Display ON:
  // Cursor OFF, blink OFF.
  lcd_command(lcd, 0x0C);
  return lcd;
}

void lcd_clear(LCD lcd) {
  // Clear display.
  lcd_command(lcd, 0x01);
  vTaskDelay(pdMS_TO_TICKS(2));
}

static void lcd_set_cursor(LCD lcd, uint8_t row, uint8_t column) {
  static const uint8_t row_offsets[] = {
      0x00,
      0x40,
      0x14,
      0x54,
  };

  if (row >= lcd.rows || column >= lcd.columns)
    return;

  lcd_command(lcd, 0x80 | (row_offsets[row] + column));
}

void lcd_print_line(LCD lcd, uint8_t row, const char *text) {
  lcd_set_cursor(lcd, row, 0);

  uint8_t i = 0;

  while (*text && i < lcd.columns) {
    lcd_data(lcd, (uint8_t)*text++);
    i++;
  }

  while (i < lcd.columns) {
    lcd_data(lcd, ' ');
    i++;
  }
}
