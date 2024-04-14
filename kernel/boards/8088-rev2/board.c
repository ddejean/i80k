// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

#include <stddef.h>

#include "delay.h"
#include "devices.h"
#include "interrupts.h"
#include "p8254.h"
#include "p8259a.h"

const struct pic pic0 = {
    .port = 0x20,
    .irq = -1.,
    .irq_base = 0,
    .irq_max = 8,
    .idt_offset = IDT_IRQ_OFFSET,
    .slave = NULL,
    .initialize = p8259a_initialize,
    .irq_enable = p8259a_irq_enable,
    .irq_disable = p8259a_irq_disable,
    .irq_ack = p8259a_irq_ack,
};

DEVICE(pic, p8259a, pic0);

struct io_device uart = {
    .port = 0x3f8,
    .irq = 4,
};

struct timer timer0 = {
    .port = 0x40,
    .irq = 0,
    .index = 0,
    .freq = 1250000U,
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
    .freq = 1250000U,
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
    .freq = 1250000U,
    .ns_per_tick = p8254_ns_per_tick,
    .read = p8254_read,
    .set_alarm = p8254_set_alarm,
    .set_freq = p8254_set_freq,
};

DEVICE(timer2, p8254, timer2);

struct memmap map = {
    .kernel =
        {
            .cs = KERNEL_CS,
            .ds = KERNEL_DS,
            .ss = KERNEL_SS,
            .stack = (void *)KERNEL_STACK_LOW,
        },
    .rom =
        {
            .segment = 0xF000,
            .count = 1,
        },
    .ram =
        {
            .segment = 0x1000,
            .count = 14,
        },
};

DEVICE(memmap, memmap, map);

void board_initialize() {
    board_register_io_dev(IO_DEV_UART, &uart);

    // Calibrate the delay loop.
    struct timer *t = timer_get("timer0");
    delay_calibrate(t);

    // Initialize the interrupt system.
    interrupts_initialize();
}
