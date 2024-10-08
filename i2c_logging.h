#ifndef I2C_LOGGING_H
#define I2C_LOGGING_H

#include <KernelExport.h>
#include <syslog.h>

// Macro per il logging
#define I2C_LOG(level, fmt, ...) do { \
    dprintf("I2C: " fmt "\n", ##__VA_ARGS__); \
    syslog(level, "I2C: " fmt, ##__VA_ARGS__); \
} while (0)

#define I2C_TRACE(fmt, ...) I2C_LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define I2C_DEBUG(fmt, ...) I2C_LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define I2C_INFO(fmt, ...)  I2C_LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define I2C_WARN(fmt, ...)  I2C_LOG(LOG_WARNING, fmt, ##__VA_ARGS__)
#define I2C_ERROR(fmt, ...) I2C_LOG(LOG_ERR, fmt, ##__VA_ARGS__)

#endif // I2C_LOGGING_H
