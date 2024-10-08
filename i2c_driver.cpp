#include "i2c_driver.h"
#include "i2c_low_level.h"
#include "i2c_transfer.h"
#include "i2c_logging.h"
#include <drivers/device_manager.h>
#include <KernelExport.h>

#define I2C_BUS_MODULE_NAME "bus_managers/i2c/driver_v1"

static device_manager_info* gDeviceManager;

static status_t
i2c_init_bus(device_node* node, void** cookie)
{
    I2C_INFO("Initializing I2C bus");
    
    i2c_bus* bus = (i2c_bus*)malloc(sizeof(i2c_bus));
    if (bus == NULL) {
        I2C_ERROR("Failed to allocate memory for i2c_bus");
        return B_NO_MEMORY;
    }

    bus->node = node;
    B_INITIALIZE_SPINLOCK(&bus->lock);

    status_t status = i2c_init_gpio();
    if (status != B_OK) {
        I2C_ERROR("Failed to initialize GPIO for I2C");
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
    {}
};

module_info* modules[] = {
    (module_info*)&sI2CDriver,
    NULL
};
