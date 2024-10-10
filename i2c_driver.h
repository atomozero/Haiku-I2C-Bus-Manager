#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <OS.h>
#include <drivers/device_manager.h>

typedef struct {
    device_node* node;
    int32 lock;  // Cambiato da spinlock a int32 per la nostra implementazione semplificata
    uint16_t scl_pin;
    uint16_t sda_pin;
    uint32_t clock_rate;
} i2c_bus;

typedef struct {
    i2c_bus* bus;
    uint8_t address;
} i2c_device;

// Dichiarazioni delle funzioni principali del driver I2C
status_t i2c_init_bus(device_node* node, void** cookie);
void i2c_uninit_bus(void* cookie);
status_t i2c_register_device(device_node* parent);

// Dichiarazioni delle funzioni di basso livello I2C
status_t i2c_init_gpio(i2c_bus* bus);
status_t i2c_uninit_gpio(i2c_bus* bus);
status_t i2c_start(i2c_bus* bus);
status_t i2c_stop(i2c_bus* bus);
status_t i2c_send_byte(i2c_bus* bus, uint8_t byte);
status_t i2c_receive_byte(i2c_bus* bus, uint8_t* byte, bool send_ack);

// Dichiarazione della struttura del driver module
extern driver_module_info gI2CDriverModule;

#endif // I2C_DRIVER_H