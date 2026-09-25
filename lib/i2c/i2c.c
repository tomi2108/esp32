#include "i2c.h"

i2c_master_bus_handle_t i2c_bus_init(gpio_num_t sda, gpio_num_t scl) {
  i2c_master_bus_handle_t bus;

  i2c_master_bus_config_t config = {
      .i2c_port = i2c_get_next_free_port(),
      .sda_io_num = sda,
      .scl_io_num = scl,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  i2c_new_master_bus(&config, &bus);
  return bus;
}

i2c_master_dev_handle_t i2c_device_init(i2c_master_bus_handle_t bus,
                                        uint8_t address, uint32_t speed_hz) {
  i2c_master_dev_handle_t device;
  i2c_device_config_t config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = address,
      .scl_speed_hz = speed_hz,
  };
  i2c_master_bus_add_device(bus, &config, &device);
  return device;
}

void i2c_write(i2c_master_dev_handle_t device, const uint8_t *data,
               size_t length) {
  i2c_master_transmit(device, data, length, -1);
}

void i2c_read(i2c_master_dev_handle_t device, uint8_t *data, size_t length) {
  i2c_master_receive(device, data, length, -1);
}

static bool used_ports[I2C_NUM_MAX];
i2c_port_t i2c_get_next_free_port(void) {
  for (i2c_port_t port = 0; port < I2C_NUM_MAX; port++) {
    if (!used_ports[port]) {
      used_ports[port] = true;
      return port;
    }
  }
  return I2C_NUM_MAX;
}
