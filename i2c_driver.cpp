#include <drivers/Drivers.h>
#include <drivers/module.h>
#include <drivers/device_manager.h>
#include <drivers/KernelExport.h>
#include <stdlib.h>

#define DRIVER_NAME "i2c"
#define I2C_BUS_MODULE_NAME "bus_managers/i2c/bus_v1"
#define I2C_DEVICE_MODULE_NAME "bus_managers/i2c/device_v1"
#define TRACE() dprintf(DRIVER_NAME ": %s\n", __PRETTY_FUNCTION__)

static device_manager_info* gDeviceManager;

typedef struct {
    device_node* node;
    uint16_t scl_pin;
    uint16_t sda_pin;
    uint32_t clock_rate;
} i2c_bus;

static status_t
i2c_init_bus(device_node* node, void** _cookie)
{
    TRACE();
    dprintf(DRIVER_NAME ": Initializing I2C bus\n");
    
    i2c_bus* bus = (i2c_bus*)malloc(sizeof(i2c_bus));
    if (bus == NULL) {
        dprintf(DRIVER_NAME ": Failed to allocate memory for i2c_bus\n");
        return B_NO_MEMORY;
    }

    // Ottieni la configurazione dal Device Manager
    if (gDeviceManager->get_attr_uint16(node, "scl_pin", &bus->scl_pin, false) != B_OK ||
        gDeviceManager->get_attr_uint16(node, "sda_pin", &bus->sda_pin, false) != B_OK ||
        gDeviceManager->get_attr_uint32(node, "clock_rate", &bus->clock_rate, false) != B_OK) {
        dprintf(DRIVER_NAME ": Failed to get I2C bus configuration\n");
        free(bus);
        return B_ERROR;
    }

    dprintf(DRIVER_NAME ": I2C bus configuration - SCL: %d, SDA: %d, Clock Rate: %lu\n", 
            bus->scl_pin, bus->sda_pin, bus->clock_rate);

    bus->node = node;

    *_cookie = bus;
    dprintf(DRIVER_NAME ": I2C bus initialized successfully\n");
    return B_OK;
}

static void
i2c_uninit_bus(void* _cookie)
{
    TRACE();
    dprintf(DRIVER_NAME ": Uninitializing I2C bus\n");
    i2c_bus* bus = (i2c_bus*)_cookie;
    free(bus);
}

static status_t
i2c_open(void* _cookie, const char* path, int openMode, void** _handle)
{
    TRACE();
    dprintf(DRIVER_NAME ": Opening I2C device\n");
    *_handle = _cookie;
    return B_OK;
}

static status_t
i2c_close(void* _cookie)
{
    TRACE();
    dprintf(DRIVER_NAME ": Closing I2C device\n");
    return B_OK;
}

static status_t
i2c_free(void* _cookie)
{
    TRACE();
    dprintf(DRIVER_NAME ": Freeing I2C device\n");
    return B_OK;
}

static status_t
i2c_read(void* _cookie, off_t position, void* buffer, size_t* _length)
{
    TRACE();
    dprintf(DRIVER_NAME ": Reading from I2C device\n");
    return B_ERROR;
}

static status_t
i2c_write(void* _cookie, off_t position, const void* buffer, size_t* _length)
{
    TRACE();
    dprintf(DRIVER_NAME ": Writing to I2C device\n");
    return B_ERROR;
}

static status_t
i2c_io(void* _cookie, io_request* request)
{
    TRACE();
    dprintf(DRIVER_NAME ": Performing I/O on I2C device\n");
    return B_ERROR;
}

static status_t
i2c_register_device(device_node* parent)
{
    TRACE();
    dprintf(DRIVER_NAME ": Registering I2C device\n");
    device_attr attrs[] = {
        { B_DEVICE_BUS, B_STRING_TYPE, { string: "i2c" }},
        { NULL }
    };

    return gDeviceManager->register_node(parent, DRIVER_NAME, attrs, NULL, NULL);
}

static float
i2c_supports_device(device_node* parent)
{
    TRACE();
    
    uint16_t vendorID;
    uint16_t deviceID;

    if (gDeviceManager->get_attr_uint16(parent, B_DEVICE_VENDOR_ID, &vendorID, false) != B_OK ||
        gDeviceManager->get_attr_uint16(parent, B_DEVICE_ID, &deviceID, false) != B_OK) {
        dprintf(DRIVER_NAME ": Failed to get vendor or device ID\n");
        return -1.0f;
    }

    dprintf(DRIVER_NAME ": Checking support for device - Vendor ID: 0x%04x, Device ID: 0x%04x\n", vendorID, deviceID);

    if (vendorID == 0x8086 && (deviceID == 0xa0e8 || deviceID == 0xa0e9)) {
        dprintf(DRIVER_NAME ": Found supported I2C controller (Vendor: 0x%04x, Device: 0x%04x)\n", vendorID, deviceID);
        return 0.8f;
    }

    dprintf(DRIVER_NAME ": Device not supported\n");
    return 0.0f;
}

static status_t
std_ops(int32 op, ...)
{
    switch (op) {
    case B_MODULE_INIT:
        dprintf(DRIVER_NAME ": Module initialization attempt\n");
        TRACE();
        dprintf(DRIVER_NAME ": Module initialized successfully\n");
        return B_OK;
    case B_MODULE_UNINIT:
        TRACE();
        dprintf(DRIVER_NAME ": Module uninitialized\n");
        return B_OK;
    default:
        dprintf(DRIVER_NAME ": Unknown operation %ld\n", op);
        return B_ERROR;
    }
}

static driver_module_info sI2CBusDriver = {
    {
        I2C_BUS_MODULE_NAME,
        0,
        std_ops
    },
    i2c_supports_device,
    i2c_register_device,
    i2c_init_bus,
    i2c_uninit_bus,
    NULL,  // remove,
    NULL,  // free,
};

static device_module_info sI2CDeviceModule = {
    {
        I2C_DEVICE_MODULE_NAME,
        0,
        std_ops
    },
    NULL,  // init_device
    NULL,  // uninit_device
    NULL,  // remove,
    i2c_open,
    i2c_close,
    i2c_free,
    i2c_read,
    i2c_write,
    i2c_io,
    NULL,  // select
    NULL   // deselect
};

module_dependency module_dependencies[] = {
    { B_DEVICE_MANAGER_MODULE_NAME, (module_info**)&gDeviceManager },
    {}
};

module_info* modules[] = {
    (module_info*)&sI2CBusDriver,
    (module_info*)&sI2CDeviceModule,
    NULL
};

__attribute__((constructor))
static void i2c_driver_load(void) {
    dprintf(DRIVER_NAME ": Driver object file loaded\n");
}

__attribute__((destructor))
static void i2c_driver_unload(void) {
    dprintf(DRIVER_NAME ": Driver object file unloaded\n");
}