// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

#include "devices.h"
#include "p8254.h"
#include "timer.h"

// PIC definition.
struct io_device pic = {
    .port = 0x20,
    .irq = -1,
};
struct io_device pic2 = {
    .port = 0xA0,
    .irq = 2,
};

struct io_device uart = {
    .port = 0x3f8,
    .irq = 4,
    .u.uart =
        {
            .freq = 4915200U,
        },
};

struct timer timer0 = {
    .port = 0x40,
    .irq = 0,
    .index = 0,
    .freq = 1193182U,
    .ns_per_tick = p8254_ns_per_tick,
    .read = p8254_read,
    .set_alarm = p8254_set_alarm,
    .set_freq = p8254_set_freq,
};

DEVICE(timer0, p8254, timer0);

struct timer timer1 = {
    .port = 0x40,
    .irq = -1,
    .index = 1,
    .freq = 1193182U,
    .ns_per_tick = p8254_ns_per_tick,
    .read = p8254_read,
    .set_alarm = p8254_set_alarm,
    .set_freq = p8254_set_freq,
};

DEVICE(timer1, p8254, timer1);

struct timer timer2 = {
    .port = 0x40,
    .irq = -1,
    .index = 2,
    .freq = 1193182U,
    .ns_per_tick = p8254_ns_per_tick,
    .read = p8254_read,
    .set_alarm = p8254_set_alarm,
    .set_freq = p8254_set_freq,
};

DEVICE(timer2, p8254, timer2);

void board_initialize() {
    board_register_io_dev(IO_DEV_PIC_MASTER, &pic);
    board_register_io_dev(IO_DEV_PIC_SLAVE, &pic2);
    board_register_io_dev(IO_DEV_UART, &uart);
}
