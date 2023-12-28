// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "devices.h"

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
