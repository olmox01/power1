/*
 * Power1 OS - Device Management Definitions
 * Hardware abstraction layer and device drivers
 */

#ifndef _DEVICES_H
#define _DEVICES_H

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

/* Device types */
#define DEVICE_TYPE_STORAGE     1
#define DEVICE_TYPE_NETWORK     2
#define DEVICE_TYPE_INPUT       3
#define DEVICE_TYPE_OUTPUT      4
#define DEVICE_TYPE_TIMER       5

/* Device status */
#define DEVICE_STATUS_UNKNOWN   0
#define DEVICE_STATUS_READY     1
#define DEVICE_STATUS_BUSY      2
#define DEVICE_STATUS_ERROR     3

/* Device structure */
struct device {
    uint32_t id;
    uint32_t type;
    uint32_t status;
    char name[32];
    void *driver_data;
    struct device_ops *ops;
    struct device *next;
};

/* Device operations */
struct device_ops {
    int (*init)(struct device *dev);
    int (*read)(struct device *dev, void *buffer, size_t size, uint64_t offset);
    int (*write)(struct device *dev, const void *buffer, size_t size, uint64_t offset);
    int (*ioctl)(struct device *dev, uint32_t cmd, void *arg);
    void (*cleanup)(struct device *dev);
};

/* Function prototypes */
int device_manager_init(void);
int device_register(struct device *dev);
int device_unregister(uint32_t device_id);
struct device *device_find_by_type(uint32_t type);
struct device *device_find_by_id(uint32_t id);

#endif /* _DEVICES_H */
