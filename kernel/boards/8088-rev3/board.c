// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

#include "devices.h"

// PIC definition.
struct io_device pic = {
    .port = 0x20,
    .irq = -1,
};

// PIT timers definition.
struct io_device timer0 = {
    .port = 0x40,
    .irq = 0,
    .u.timer =
        {
            .index = 0,
            .freq = 2457600U,
        },
};

struct io_device timer1 = {
    .port = 0x40,
    .irq = -1,
    .u.timer =
        {
            .index = 1,
            .freq = 2457600U,
        },
};

struct io_device timer2 = {
    .port = 0x40,
    .irq = -1,
    .u.timer =
        {
            .index = 2,
            .freq = 2457600U,
        },
};

struct io_device uart = {
    .port = 0x3f8,
    .irq = 4,
    .u.uart =
        {
            .freq = 4915200U,
        },
};

struct io_device cf = {
    .port = 0x1f0,
    .irq = 5,
    .u.cf =
        {
            .is_8bit = true,
        },
};

void board_initialize() {
    board_register_io_dev(IO_DEV_PIC_MASTER, &pic);
    board_register_io_dev(IO_DEV_PIT_TIMER0, &timer0);
    board_register_io_dev(IO_DEV_PIT_TIMER1, &timer1);
    board_register_io_dev(IO_DEV_PIT_TIMER2, &timer2);
    board_register_io_dev(IO_DEV_UART, &uart);
    board_register_io_dev(IO_DEV_CF, &cf);
}
