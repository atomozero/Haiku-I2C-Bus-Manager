#ifndef I2C_LOW_LEVEL_H
#define I2C_LOW_LEVEL_H

#include "i2c_driver.h"

status_t i2c_init_gpio();
status_t i2c_start(i2c_bus* bus);
status_t i2c_stop(i2c_bus* bus);
status_t i2c_send_byte(i2c_bus* bus, uint8_t byte);
status_t i2c_receive_byte(i2c_bus* bus, uint8_t* byte, bool send_ack);

#endif // I2C_LOW_LEVEL_H
