#include "i2c_low_level.h"
#include "i2c_logging.h"
#include <KernelExport.h>
#include <atomic>

// Implementazione semplificata di spinlock usando atomic
static inline void acquire_spinlock(int32* lock) {
    while (atomic_add(lock, 1) != 0) {
        atomic_add(lock, -1);
        snooze(1);
    }
}

static inline void release_spinlock(int32* lock) {
    atomic_add(lock, -1);
}

// Implementazione stub per le funzioni GPIO
static void set_pin(uint16_t pin, bool value) {
    I2C_TRACE("GPIO: Setting pin %d to %d", pin, value);
    // Implementazione reale da aggiungere in futuro
}

static bool get_pin(uint16_t pin) {
    I2C_TRACE("GPIO: Reading pin %d", pin);
    // Implementazione reale da aggiungere in futuro
    return false;
}

static status_t configure_pin(uint16_t pin, uint32_t config) {
    I2C_TRACE("GPIO: Configuring pin %d with config %u", pin, config);
    // Implementazione reale da aggiungere in futuro
    return B_OK;
}

status_t
i2c_init_gpio(i2c_bus* bus)
{
    I2C_DEBUG("Initializing GPIO for I2C: SCL pin %d, SDA pin %d", bus->scl_pin, bus->sda_pin);
    
    status_t status = configure_pin(bus->scl_pin, 0); // Usando 0 come configurazione di default
    if (status != B_OK) {
        I2C_ERROR("Failed to configure SCL pin %d", bus->scl_pin);
        return status;
    }

    status = configure_pin(bus->sda_pin, 0); // Usando 0 come configurazione di default
    if (status != B_OK) {
        I2C_ERROR("Failed to configure SDA pin %d", bus->sda_pin);
        return status;
    }

    set_pin(bus->scl_pin, true);
    set_pin(bus->sda_pin, true);

    I2C_INFO("GPIO initialized for I2C");
    return B_OK;
}

status_t
i2c_uninit_gpio(i2c_bus* bus)
{
    I2C_DEBUG("Uninitializing GPIO for I2C: SCL pin %d, SDA pin %d", bus->scl_pin, bus->sda_pin);
    
    configure_pin(bus->scl_pin, 0);
    configure_pin(bus->sda_pin, 0);

    I2C_INFO("GPIO uninitialized for I2C");
    return B_OK;
}

static inline void set_scl(i2c_bus* bus, bool value) {
    set_pin(bus->scl_pin, value);
    I2C_TRACE("SCL (pin %d) set to %d", bus->scl_pin, value);
}

static inline void set_sda(i2c_bus* bus, bool value) {
    set_pin(bus->sda_pin, value);
    I2C_TRACE("SDA (pin %d) set to %d", bus->sda_pin, value);
}

static inline bool read_sda(i2c_bus* bus) {
    bool value = get_pin(bus->sda_pin);
    I2C_TRACE("SDA (pin %d) read: %d", bus->sda_pin, value);
    return value;
}

static inline void i2c_delay(i2c_bus* bus) {
    int delay_us = 1000000 / (bus->clock_rate * 2);
    snooze(delay_us);
    I2C_TRACE("Delay %d µs", delay_us);
}

status_t
i2c_start(i2c_bus* bus)
{
    I2C_DEBUG("Generating START condition");
    cpu_status cpu_state = disable_interrupts();
    acquire_spinlock(&bus->lock);

    set_sda(bus, true);
    i2c_delay(bus);
    set_scl(bus, true);
    i2c_delay(bus);
    set_sda(bus, false);
    i2c_delay(bus);
    set_scl(bus, false);
    i2c_delay(bus);

    release_spinlock(&bus->lock);
    restore_interrupts(cpu_state);
    I2C_DEBUG("START condition generated");
    return B_OK;
}

status_t
i2c_stop(i2c_bus* bus)
{
    I2C_DEBUG("Generating STOP condition");
    cpu_status cpu_state = disable_interrupts();
    acquire_spinlock(&bus->lock);

    set_sda(bus, false);
    i2c_delay(bus);
    set_scl(bus, true);
    i2c_delay(bus);
    set_sda(bus, true);
    i2c_delay(bus);

    release_spinlock(&bus->lock);
    restore_interrupts(cpu_state);
    I2C_DEBUG("STOP condition generated");
    return B_OK;
}

status_t
i2c_send_byte(i2c_bus* bus, uint8_t byte)
{
    I2C_DEBUG("Sending byte: 0x%02x", byte);
    status_t status;
    cpu_status cpu_state = disable_interrupts();
    acquire_spinlock(&bus->lock);

    for (int i = 7; i >= 0; i--) {
        set_sda(bus, (byte >> i) & 1);
        i2c_delay(bus);
        set_scl(bus, true);
        i2c_delay(bus);
        set_scl(bus, false);
        i2c_delay(bus);
    }

    // Leggi ACK
    set_sda(bus, true);
    i2c_delay(bus);
    set_scl(bus, true);
    i2c_delay(bus);
    bool ack = !read_sda(bus);
    set_scl(bus, false);
    i2c_delay(bus);

    status = ack ? B_OK : B_ERROR;

    release_spinlock(&bus->lock);
    restore_interrupts(cpu_state);

    if (status == B_OK) {
        I2C_DEBUG("Byte sent successfully, ACK received");
    } else {
        I2C_WARN("Byte send failed, NACK received");
    }

    return status;
}

status_t
i2c_receive_byte(i2c_bus* bus, uint8_t* byte, bool send_ack)
{
    I2C_DEBUG("Receiving byte, will send %s", send_ack ? "ACK" : "NACK");
    cpu_status cpu_state = disable_interrupts();
    acquire_spinlock(&bus->lock);

    *byte = 0;
    set_sda(bus, true);  // Rilascia la linea SDA

    for (int i = 7; i >= 0; i--) {
        set_scl(bus, true);
        i2c_delay(bus);
        *byte |= (read_sda(bus) ? 1 : 0) << i;
        set_scl(bus, false);
        i2c_delay(bus);
    }

    // Invia ACK/NACK
    set_sda(bus, !send_ack);
    i2c_delay(bus);
    set_scl(bus, true);
    i2c_delay(bus);
    set_scl(bus, false);
    i2c_delay(bus);
    set_sda(bus, true);  // Rilascia la linea SDA

    release_spinlock(&bus->lock);
    restore_interrupts(cpu_state);

    I2C_DEBUG("Byte received: 0x%02x, sent %s", *byte, send_ack ? "ACK" : "NACK");
    return B_OK;
}