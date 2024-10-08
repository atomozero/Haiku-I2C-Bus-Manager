#include "i2c_low_level.h"
#include "i2c_logging.h"
#include <drivers/gpio.h>

#define I2C_SCL_GPIO 2  // GPIO pin per SCL
#define I2C_SDA_GPIO 3  // GPIO pin per SDA
#define I2C_DELAY_US 5  // Ritardo in microsecondi per il timing I2C

static gpio_module_info* gGPIO;

static inline void set_scl(bool value) {
    gGPIO->set_pin(I2C_SCL_GPIO, value);
    I2C_TRACE("SCL set to %d", value);
}

static inline void set_sda(bool value) {
    gGPIO->set_pin(I2C_SDA_GPIO, value);
    I2C_TRACE("SDA set to %d", value);
}

static inline bool read_sda() {
    bool value = gGPIO->get_pin(I2C_SDA_GPIO);
    I2C_TRACE("SDA read: %d", value);
    return value;
}

static inline void i2c_delay() {
    snooze(I2C_DELAY_US);
    I2C_TRACE("Delay %d µs", I2C_DELAY_US);
}

status_t
i2c_init_gpio()
{
    I2C_DEBUG("Initializing GPIO for I2C");
    status_t status;

    status = get_module(B_GPIO_MODULE_NAME, (module_info**)&gGPIO);
    if (status != B_OK) {
        I2C_ERROR("Failed to get GPIO module: %s", strerror(status));
        return status;
    }

    status = gGPIO->configure_pin(I2C_SCL_GPIO, GPIO_PIN_OUTPUT | GPIO_PIN_OPEN_DRAIN);
    if (status != B_OK) {
        I2C_ERROR("Failed to configure SCL pin: %s", strerror(status));
        return status;
    }

    status = gGPIO->configure_pin(I2C_SDA_GPIO, GPIO_PIN_OUTPUT | GPIO_PIN_OPEN_DRAIN);
    if (status != B_OK) {
        I2C_ERROR("Failed to configure SDA pin: %s", strerror(status));
        return status;
    }

    set_scl(true);
    set_sda(true);

    I2C_INFO("GPIO initialized for I2C");
    return B_OK;
}

status_t
i2c_start(i2c_bus* bus)
{
    I2C_DEBUG("Generating START condition");
    cpu_status cpu_state = disable_interrupts();
    acquire_spinlock(&bus->lock);

    set_sda(true);
    i2c_delay();
    set_scl(true);
    i2c_delay();
    set_sda(false);
    i2c_delay();
    set_scl(false);
    i2c_delay();

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

    set_sda(false);
    i2c_delay();
    set_scl(true);
    i2c_delay();
    set_sda(true);
    i2c_delay();

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
        set_sda((byte >> i) & 1);
        i2c_delay();
        set_scl(true);
        i2c_delay();
        set_scl(false);
        i2c_delay();
    }

    // Leggi ACK
    set_sda(true);
    i2c_delay();
    set_scl(true);
    i2c_delay();
    bool ack = !read_sda();
    set_scl(false);
    i2c_delay();

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
    set_sda(true);  // Rilascia la linea SDA

    for (int i = 7; i >= 0; i--) {
        set_scl(true);
        i2c_delay();
        *byte |= (read_sda() ? 1 : 0) << i;
        set_scl(false);
        i2c_delay();
    }

    // Invia ACK/NACK
    set_sda(!send_ack);
    i2c_delay();
    set_scl(true);
    i2c_delay();
    set_scl(false);
    i2c_delay();
    set_sda(true);  // Rilascia la linea SDA

    release_spinlock(&bus->lock);
    restore_interrupts(cpu_state);

    I2C_DEBUG("Byte received: 0x%02x, sent %s", *byte, send_ack ? "ACK" : "NACK");
    return B_OK;
}
