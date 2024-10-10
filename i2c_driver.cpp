#include "i2c_driver.h"
#include "i2c_low_level.h"
#include "i2c_logging.h"
#include <drivers/device_manager.h>
#include <KernelExport.h>
#include <stdlib.h>

#define I2C_BUS_MODULE_NAME "bus_managers/i2c/driver_v1"

static device_manager_info* gDeviceManager;

static float
i2c_supports_device(device_node* parent)
{
    // Implementa la logica per determinare se questo driver supporta il dispositivo
    // Per ora, assumiamo che supporti sempre il dispositivo
    return 0.8f;
}

status_t
i2c_register_device(device_node* parent)
{
    I2C_INFO("Registering I2C device");
    device_attr attrs[] = {
        { B_DEVICE_BUS, B_STRING_TYPE, { string: "i2c" }},
        { NULL }
    };

    return gDeviceManager->register_node(parent, I2C_BUS_MODULE_NAME, attrs, NULL, NULL);
}

status_t
i2c_init_bus(device_node* node, void** cookie)
{
    I2C_INFO("Initializing I2C bus");
    
    i2c_bus* bus = (i2c_bus*)malloc(sizeof(i2c_bus));
    if (bus == NULL) {
        I2C_ERROR("Failed to allocate memory for i2c_bus");
        return B_NO_MEMORY;
    }

    // Ottenere la configurazione dal Device Manager
    if (gDeviceManager->get_attr_uint16(node, "scl_pin", &bus->scl_pin, false) != B_OK ||
        gDeviceManager->get_attr_uint16(node, "sda_pin", &bus->sda_pin, false) != B_OK ||
        gDeviceManager->get_attr_uint32(node, "clock_rate", &bus->clock_rate, false) != B_OK) {
        I2C_ERROR("Failed to get I2C bus configuration");
        free(bus);
        return B_ERROR;
    }

    bus->node = node;
    bus->lock = 0;  // Inizializza il lock a 0

    status_t status = i2c_init_gpio(bus);
    if (status != B_OK) {
        I2C_ERROR("Failed to initialize GPIO for I2C");
        free(bus);
        return status;
    }

    *cookie = bus;
    I2C_INFO("I2C bus initialized successfully");
    return B_OK;
}

void
i2c_uninit_bus(void* cookie)
{
    i2c_bus* bus = (i2c_bus*)cookie;
    I2C_INFO("Uninitializing I2C bus");
    
    i2c_uninit_gpio(bus);
    free(bus);
}

static status_t
i2c_register_child_devices(void* cookie)
{
    // Implementa la logica per registrare eventuali dispositivi figli
    // Per ora, non facciamo nulla
    return B_OK;
}

static driver_module_info sI2CDriver = {
    {
        I2C_BUS_MODULE_NAME,
        0,
        NULL
    },
    i2c_supports_device,       // Funzione per determinare se il driver supporta il dispositivo
    i2c_register_device,       // Funzione per registrare il dispositivo
    i2c_init_bus,              // Funzione di inizializzazione del driver
    i2c_uninit_bus,            // Funzione di deinizializzazione del driver
    i2c_register_child_devices,// Funzione per registrare dispositivi figli (se necessario)
    NULL,                      // rescan_child_devices (non implementato)
    NULL                       // free_cookie (non necessario se usiamo free() standard)
};

module_dependency module_dependencies[] = {
    { B_DEVICE_MANAGER_MODULE_NAME, (module_info**)&gDeviceManager },
    {}
};

module_info* modules[] = {
    (module_info*)&sI2CDriver,
    NULL
};