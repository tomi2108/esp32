#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include <stddef.h>
#include <stdint.h>

i2c_master_bus_handle_t i2c_bus_init(gpio_num_t sda, gpio_num_t scl);
i2c_master_dev_handle_t i2c_device_init(i2c_master_bus_handle_t bus,
                                        uint8_t address, uint32_t speed_hz);

void i2c_write(i2c_master_dev_handle_t device, const uint8_t *data,
               size_t length);
void i2c_read(i2c_master_dev_handle_t device, uint8_t *data, size_t length);
i2c_port_t i2c_get_next_free_port(void);
