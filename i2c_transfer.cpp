#include "i2c_transfer.h"
#include "i2c_low_level.h"
#include "i2c_logging.h"
#include <KernelExport.h>

#define I2C_RETRY_COUNT 3
#define I2C_TIMEOUT 1000000  // 1 secondo in microsecondi

static const char* status_to_string(status_t status) {
    switch (status) {
        case B_OK: return "B_OK";
        case B_ERROR: return "B_ERROR";
        case B_TIMED_OUT: return "B_TIMED_OUT";
        case B_BAD_VALUE: return "B_BAD_VALUE";
        default: return "Unknown";
    }
}

status_t
i2c_transfer(i2c_bus* bus, i2c_device* device, i2c_transfer_data* data)
{
    if (bus == NULL || device == NULL || data == NULL) {
        I2C_ERROR("Invalid parameters");
        return B_BAD_VALUE;
    }

    status_t status = B_OK;
    uint32_t retries = I2C_RETRY_COUNT;

    I2C_DEBUG("Starting I2C transfer: device_addr=0x%02x, %s, length=%d",
              device->address, (data->flags & I2C_TRANSFER_READ) ? "read" : "write", data->length);

    while (retries--) {
        bigtime_t start_time = system_time();

        I2C_TRACE("Attempt %d", I2C_RETRY_COUNT - retries);

        // Inizia la transazione
        status = i2c_start(bus);
        if (status != B_OK) {
            I2C_WARN("Failed to start I2C transaction: %s", status_to_string(status));
            continue;
        }

        // Invia l'indirizzo del dispositivo
        uint8_t address = (device->address << 1) | (data->flags & I2C_TRANSFER_READ ? 1 : 0);
        status = i2c_send_byte(bus, address);
        if (status != B_OK) {
            I2C_WARN("Failed to send device address 0x%02x: %s", address, status_to_string(status));
            i2c_stop(bus);
            continue;
        }

        if (data->flags & I2C_TRANSFER_READ) {
            // Operazione di lettura
            for (uint32_t i = 0; i < data->length; i++) {
                status = i2c_receive_byte(bus, &data->data[i], i < data->length - 1);
                if (status != B_OK) {
                    I2C_WARN("Failed to receive byte %d: %s", i, status_to_string(status));
                    break;
                }

                if (system_time() - start_time > I2C_TIMEOUT) {
                    I2C_ERROR("Timeout during read operation");
                    status = B_TIMED_OUT;
                    break;
                }
            }
        } else {
            // Operazione di scrittura
            for (uint32_t i = 0; i < data->length; i++) {
                status = i2c_send_byte(bus, data->data[i]);
                if (status != B_OK) {
                    I2C_WARN("Failed to send byte %d (0x%02x): %s", i, data->data[i], status_to_string(status));
                    break;
                }

                if (system_time() - start_time > I2C_TIMEOUT) {
                    I2C_ERROR("Timeout during write operation");
                    status = B_TIMED_OUT;
                    break;
                }
            }
        }

        // Termina la transazione
        i2c_stop(bus);

        if (status == B_OK) {
            I2C_DEBUG("I2C transfer completed successfully");
            break;  // Trasferimento completato con successo
        }

        // Se abbiamo un timeout, non ha senso riprovare
        if (status == B_TIMED_OUT) {
            I2C_ERROR("I2C transfer timed out, aborting");
            break;
        }

I2C_INFO("Retrying I2C transfer...");
        // Breve pausa prima di riprovare
        snooze(1000);  // 1 millisecondo
    }

    if (status != B_OK) {
        I2C_ERROR("I2C transfer failed after %d attempts: %s", I2C_RETRY_COUNT, status_to_string(status));
    }

    return status;
}
