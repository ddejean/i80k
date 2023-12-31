// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

#include "devices.h"

// PIC definition.
struct io_device pic = {
    .port = 0x20,
};

// PIT timers definition.
struct io_device timer0 = {
    .port = 0x40,
    .u.timer =
        {
            .index = 0,
            .irq = 0,
            .freq = 2457600U,
        },
};

struct io_device timer1 = {
    .port = 0x40,
    .u.timer =
        {
            .index = 1,
            .irq = -1,
            .freq = 2457600U,
        },
};

struct io_device timer2 = {
    .port = 0x40,
    .u.timer =
        {
            .index = 2,
            .irq = -1,
            .freq = 2457600U,
        },
};

void board_initialize() {
    board_register_io_dev(IO_DEV_PIC_MASTER, &pic);
    board_register_io_dev(IO_DEV_PIT_TIMER0, &timer0);
    board_register_io_dev(IO_DEV_PIT_TIMER1, &timer1);
    board_register_io_dev(IO_DEV_PIT_TIMER2, &timer2);
}
