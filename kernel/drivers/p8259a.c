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

// I/O device used by the driver.
static struct io_device *pic;

static inline void p8259a_initialize(uint8_t offset) {
    // Get the PIC defition from the devices declared by the board.
    pic = board_get_io_dev(IO_DEV_PIC_MASTER);

    // First configuration word (ICW1):
    // - Level triggered (not edge triggered)
    // - Interrupts handlers entries are 4 bytes wide (segment + offset)
    // - Single PIC
    outb(PIC_CMD(pic), ICW1_INIT | ICW1_INTERVAL4 | ICW1_SINGLE | ICW1_ICW4);
    // Second configuration word (ICW2): offset in interrupts table. 8 is the
    // classical offset for IBM PC/XT.
    outb(PIC_DATA(pic), offset);
    // Fourth configuration word (ICW4): 8086 mode.
    outb(PIC_DATA(pic), ICW4_8086);
    // Mask all interrupts.
    outb(PIC_DATA(pic), MASK_ALL);
}

static inline void p8259a_enable(uint8_t mask) {
    uint8_t value = inb(PIC_DATA(pic));
    value &= ~mask;
    outb(PIC_DATA(pic), value);
}

static inline void p8259a_disable(uint8_t mask) {
    uint8_t value = inb(PIC_DATA(pic));
    value |= mask;
    outb(PIC_DATA(pic), value);
}

static inline void p8259a_ack(void) { outb(PIC_CMD(pic), 0x20); }

void irq_setup(void) {
    // Configure the PIC for the board configuration and the offset in the
    // interrupt table.
    p8259a_initialize(IDT_IRQ_OFFSET);
}

void irq_enable(uint8_t mask) { p8259a_enable(mask); }

void irq_disable(uint8_t mask) { p8259a_disable(mask); }

void irq_ack(void) { p8259a_ack(); }
