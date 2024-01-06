// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>

#include "board.h"
#include "cpu.h"
#include "devices.h"
#include "irq.h"

// PIC ports.
#define PIC_CMD(dev) (dev->port)
#define PIC_DATA(dev) (dev->port + 1)

// PIC command words
#define ICW1_ICW4 0x01       // ICW4 (not) needed
#define ICW1_SINGLE 0x02     // Single (cascade) mode
#define ICW1_INTERVAL4 0x04  // Call address interval 4 (8)
#define ICW1_LEVEL 0x08      // Level triggered (edge) mode
#define ICW1_INIT 0x10       // Initialization bit

#define ICW4_8086 0x01        // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO 0x02        // Auto (normal) EOI
#define ICW4_BUF_SLAVE 0x08   // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C  // Buffered mode/master
#define ICW4_SFNM 0x10        // Special fully nested (not)

// I/O devices used by the driver.
static struct io_device *pic;
static struct io_device *pic_slave;

static inline void p8259a_initialize(uint8_t offset) {
    // Get the PIC defition from the devices declared by the board.
    pic = board_get_io_dev(IO_DEV_PIC_MASTER);
    pic_slave = board_get_io_dev(IO_DEV_PIC_SLAVE);

    // First configuration word (ICW1):
    // - Level triggered (not edge triggered)
    // - Interrupts handlers entries are 4 bytes wide (segment + offset)
    if (pic_slave) {
        outb(PIC_CMD(pic), ICW1_INIT | ICW1_INTERVAL4 | ICW1_ICW4);
    } else {
        outb(PIC_CMD(pic),
             ICW1_INIT | ICW1_INTERVAL4 | ICW1_SINGLE | ICW1_ICW4);
    }
    // Second configuration word (ICW2): offset in interrupts table.
    outb(PIC_DATA(pic), offset);
    // Third configuration word (ICW3): IRQ where the slave PIC is wired.
    if (pic_slave) {
        outb(PIC_DATA(pic), 1 << pic_slave->irq);
    }
    // Fourth configuration word (ICW4): 8086 mode.
    outb(PIC_DATA(pic), ICW4_8086);
    // Mask all interrupts.
    outb(PIC_DATA(pic), 0xff);

    // There's a second PIC.
    if (pic_slave) {
        outb(PIC_CMD(pic_slave), ICW1_INIT | ICW1_INTERVAL4 | ICW1_ICW4);
        outb(PIC_DATA(pic_slave), offset + 8);
        outb(PIC_DATA(pic_slave), pic_slave->irq);
        outb(PIC_DATA(pic_slave), ICW4_8086);
        outb(PIC_DATA(pic_slave), 0xff);
    }
}

static inline void p8259a_enable(int irq) {
    uint8_t mask;
    uint8_t value;

    if (irq < 8) {
        mask = 1 << irq;
    } else {
        mask = 1 << (irq - 8);
    }

    if (irq < 8) {
        value = inb(PIC_DATA(pic));
        value &= ~mask;
        outb(PIC_DATA(pic), value);
    } else if (irq >= 8 && pic_slave) {
        value = inb(PIC_DATA(pic_slave));
        value &= ~mask;
        outb(PIC_DATA(pic_slave), value);
    }
}

static inline void p8259a_disable(int irq) {
    uint8_t mask;
    uint8_t value;

    if (irq < 8) {
        mask = 1 << irq;
    } else {
        mask = 1 << (irq - 8);
    }

    if (irq < 8) {
        value = inb(PIC_DATA(pic));
        value |= mask;
        outb(PIC_DATA(pic), value);
    } else if (irq >= 8 && pic_slave) {
        value = inb(PIC_DATA(pic_slave));
        value |= mask;
        outb(PIC_DATA(pic_slave), value);
    }
}

static inline void p8259a_ack(int irq) {
    if (irq >= 8) {
        outb(PIC_CMD(pic_slave), 0x20);
    }
    outb(PIC_CMD(pic), 0x20);
}

void irq_setup(void) {
    // Configure the PIC for the board configuration and the offset in the
    // interrupt table.
    p8259a_initialize(IDT_IRQ_OFFSET);
}

void irq_enable(int irq) { p8259a_enable(irq); }

void irq_disable(int irq) { p8259a_disable(irq); }

void irq_ack(int irq) { p8259a_ack(irq); }
