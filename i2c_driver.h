#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <OS.h>
#include <drivers/device_manager.h>

typedef struct {
    device_node* node;
    spinlock lock;
} i2c_bus;

typedef struct {
    i2c_bus* bus;
    uint8_t address;
} i2c_device;

#endif // I2C_DRIVER_H
