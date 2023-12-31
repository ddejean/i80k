// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <stdint.h>

// IO devices categories.
typedef enum _io_device_t {
    IO_DEV_PIC_MASTER = 0,
    IO_DEV_PIT_TIMER0,
    IO_DEV_PIT_TIMER1,
    IO_DEV_PIT_TIMER2,
    IO_DEV_MAX,
} io_device_t;

struct io_device {
    uint16_t port;
    union {
        struct {
            int index;
            int irq;
            unsigned long freq;
        } timer;
    } u;
};

// board_register_io_dev registers an I/O device in the devices set. Returns 0
// on success, -1 on error.
int board_register_io_dev(io_device_t type, struct io_device *dev);

// board_get_io_dev obtains the I/O device matching <type> if registered, NULL
// otherwise.
struct io_device *board_get_io_dev(io_device_t type);

#endif  // _DEVICES_H_
