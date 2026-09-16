#include "lcd.h"

void lcd_write_byte(LCD lcd, uint8_t data) {
  i2c_master_transmit(lcd.device, &data, 1, -1);
}

void lcd_pulse_enable(LCD lcd, uint8_t data) {
  lcd_write_byte(lcd, data | LCD_ENABLE);
  esp_rom_delay_us(1);
  lcd_write_byte(lcd, data & ~LCD_ENABLE);
  esp_rom_delay_us(50);
}

void lcd_write_nibble(LCD lcd, uint8_t nibble, uint8_t control) {
  uint8_t data = (nibble & 0xF0) | control | LCD_BACKLIGHT;
  lcd_pulse_enable(lcd, data);
}

void lcd_send(LCD lcd, uint8_t value, uint8_t control) {
  lcd_write_nibble(lcd, value & 0xF0, control);
  lcd_write_nibble(lcd, (value << 4) & 0xF0, control);
}

void lcd_command(LCD lcd, uint8_t command) { lcd_send(lcd, command, 0); }
void lcd_data(LCD lcd, uint8_t data) { lcd_send(lcd, data, LCD_REG_SELECT); }
void lcd_clear(LCD lcd) {
  lcd_command(lcd, 0x01);
  vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_set_cursor(LCD lcd, uint8_t row, uint8_t column) {
  uint8_t address;
  if (row == 0)
    address = 0x00 + column;
  else if (row == 1)
    address = 0x40 + column;
  else
    return;

  lcd_command(lcd, 0x80 | address);
}

void lcd_print_line(LCD lcd, uint8_t row, const char *text) {
  lcd_set_cursor(lcd, row, 0);
  while (*text)
    lcd_data(lcd, (uint8_t)*text++);
}

LCD lcd_init(gpio_num_t sda, gpio_num_t scl) {
  LCD lcd = {.sda = sda, .scl = scl};
  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = sda,
      .scl_io_num = scl,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };

  i2c_device_config_t dev_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = LCD_ADDR,
      .scl_speed_hz = 100000,
  };

  i2c_new_master_bus(&bus_config, &lcd.bus);
  i2c_master_bus_add_device(lcd.bus, &dev_config, &lcd.device);

  vTaskDelay(pdMS_TO_TICKS(50));
  lcd_write_nibble(lcd, 0x30, 0);
  esp_rom_delay_us(150);
  lcd_write_nibble(lcd, 0x30, 0);
  esp_rom_delay_us(150);
  lcd_write_nibble(lcd, 0x30, 0);
  esp_rom_delay_us(150);

  lcd_write_nibble(lcd, 0x20, 0);

  lcd_command(lcd, 0x28);
  lcd_command(lcd, 0x0C);
  lcd_command(lcd, 0x06);
  lcd_clear(lcd);
  return lcd;
}
