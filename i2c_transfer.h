#ifndef I2C_TRANSFER_H
#define I2C_TRANSFER_H

#include "i2c_driver.h"

#define I2C_TRANSFER_READ 0x01

typedef struct {
    uint8_t* data;
    size_t length;
    uint32_t flags;
} i2c_transfer_data;

status_t i2c_transfer(i2c_bus* bus, i2c_device* device, i2c_transfer_data* data);

#endif // I2C_TRANSFER_H
