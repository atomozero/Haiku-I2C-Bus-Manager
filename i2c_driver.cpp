#include "i2c_driver.h"
#include "i2c_low_level.h"
#include "i2c_transfer.h"
#include "i2c_logging.h"
#include <drivers/device_manager.h>
#include <drivers/KernelExport.h>
#include <drivers/power_management.h>

#define I2C_BUS_MODULE_NAME "bus_managers/i2c/driver_v1"

static device_manager_info* gDeviceManager;
static gpio_module_info* gGPIO;

// Struttura per la configurazione del bus I2C
typedef struct {
    uint16_t scl_pin;
    uint16_t sda_pin;
    uint32_t clock_rate;
} i2c_bus_config;

static status_t
i2c_init_bus(device_node* node, void** cookie)
{
    I2C_INFO("Initializing I2C bus");
    
    i2c_bus* bus = (i2c_bus*)malloc(sizeof(i2c_bus));
    if (bus == NULL) {
        I2C_ERROR("Failed to allocate memory for i2c_bus");
        return B_NO_MEMORY;
    }

    // Ottenere la configurazione dal Device Manager
    i2c_bus_config config;
    if (gDeviceManager->get_attr_uint16(node, "scl_pin", &config.scl_pin, false) != B_OK ||
        gDeviceManager->get_attr_uint16(node, "sda_pin", &config.sda_pin, false) != B_OK ||
        gDeviceManager->get_attr_uint32(node, "clock_rate", &config.clock_rate, false) != B_OK) {
        I2C_ERROR("Failed to get I2C bus configuration");
        free(bus);
        return B_ERROR;
    }

    bus->node = node;
    bus->scl_pin = config.scl_pin;
    bus->sda_pin = config.sda_pin;
    bus->clock_rate = config.clock_rate;
    B_INITIALIZE_SPINLOCK(&bus->lock);

    status_t status = i2c_init_gpio(bus);
    if (status != B_OK) {
        I2C_ERROR("Failed to initialize GPIO for I2C");
        free(bus);
        return status;
    }

    // Registrare il bus per la gestione dell'alimentazione
    status = register_device_power_handler(node, i2c_power_management, bus);
    if (status != B_OK) {
        I2C_ERROR("Failed to register power management handler");
        i2c_uninit_gpio(bus);
        free(bus);
        return status;
    }

    *cookie = bus;
    I2C_INFO("I2C bus initialized successfully");
    return B_OK;
}

static void
i2c_uninit_bus(void* cookie)
{
    i2c_bus* bus = (i2c_bus*)cookie;
    I2C_INFO("Uninitializing I2C bus");
    
    unregister_device_power_handler(bus->node);
    i2c_uninit_gpio(bus);
    free(bus);
}

static status_t
i2c_register_device(device_node* parent)
{
    I2C_INFO("Registering I2C device");
    device_attr attrs[] = {
        { B_DEVICE_BUS, B_STRING_TYPE, { string: "i2c" }},
        { NULL }
    };

    return gDeviceManager->register_node(parent, I2C_BUS_MODULE_NAME, attrs, NULL, NULL);
}

static status_t
i2c_power_management(void* cookie, power_management_event event)
{
    i2c_bus* bus = (i2c_bus*)cookie;
    switch (event) {
        case kPowerManagementSuspend:
            // Implementa qui la logica per la sospensione
            break;
        case kPowerManagementResume:
            // Implementa qui la logica per la ripresa
            break;
        default:
            return B_ERROR;
    }
    return B_OK;
}

static driver_module_info sI2CDriver = {
    {
        I2C_BUS_MODULE_NAME,
        0,
        NULL
    },
    i2c_register_device,
    i2c_init_bus,
    i2c_uninit_bus,
    NULL, // supportChild
    NULL  // rescan
};

module_dependency module_dependencies[] = {
    { B_DEVICE_MANAGER_MODULE_NAME, (module_info**)&gDeviceManager },
    { B_GPIO_MODULE_NAME, (module_info**)&gGPIO },
    {}
};

module_info* modules[] = {
    (module_info*)&sI2CDriver,
    NULL
};
