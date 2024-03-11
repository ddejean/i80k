// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

#include <stddef.h>

#include "delay.h"
#include "devices.h"
#include "interrupts.h"
#include "p8254.h"
#include "p8259a.h"
#include "timer.h"

const struct pic pic1 = {
    .port = 0xA0,
    .irq = 2,
    .irq_base = 8,
    .irq_max = 16,
    .idt_offset = IDT_IRQ_OFFSET + 8,
    .slave = NULL,
    .initialize = p8259a_initialize,
    .irq_enable = p8259a_irq_enable,
    .irq_disable = p8259a_irq_disable,
    .irq_ack = p8259a_irq_ack,
};

const struct pic pic0 = {
    .port = 0x20,
    .irq = -1,
    .irq_base = 0,
    .irq_max = 8,
    .idt_offset = IDT_IRQ_OFFSET,
    .slave = &pic1,
    .initialize = p8259a_initialize,
    .irq_enable = p8259a_irq_enable,
    .irq_disable = p8259a_irq_disable,
    .irq_ack = p8259a_irq_ack,
};

DEVICE(pic, p8259a, pic0);

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
    board_register_io_dev(IO_DEV_UART, &uart);

    // Calibrate the delay loop.
    struct timer *t = timer_get("timer0");
    delay_calibrate(t);

    // Initialize the interrupt system.
    interrupts_initialize();
}
