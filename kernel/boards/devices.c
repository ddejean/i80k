// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "devices.h"

#include <string.h>

// Set of devices registered for the current board.
struct io_device *devices[IO_DEV_MAX];

int board_register_io_dev(io_device_t type, struct io_device *dev) {
    if (devices[type]) {
        return -1;
    }
    devices[type] = dev;
    return 0;
}

struct io_device *board_get_io_dev(io_device_t type) { return devices[type]; }

// Start of the devices declarations array, filled by the linker.
extern const struct device _devices_start[];
// End of the driver's declarations array, filled by the linker.
extern const struct device _devices_end[];

const struct device *board_get_by_driver(const char *name) {
    if (!name) {
        return NULL;
    }
    for (const struct device *dev = _devices_start; dev != _devices_end;
         dev++) {
        if (!strcmp(name, dev->driver)) {
            return dev;
        }
    }
    return NULL;
}