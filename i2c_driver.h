#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <OS.h>
#include <drivers/device_manager.h>

typedef struct {
    device_node* node;
    spinlock lock;
    uint16_t scl_pin;
    uint16_t sda_pin;
    uint32_t clock_rate;
} i2c_bus;

typedef struct {
    i2c_bus* bus;
    uint8_t address;
} i2c_device;

// Dichiarazioni delle funzioni di gestione dell'alimentazione
status_t i2c_power_management(void* cookie, power_management_event event);

#endif // I2C_DRIVER_H
