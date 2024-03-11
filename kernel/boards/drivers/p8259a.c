// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>

#include "cpu.h"
#include "devices.h"
#include "interrupts.h"

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

void p8259a_initialize(const struct pic *p) {
    // First configuration word (ICW1):
    // - Level triggered (not edge triggered)
    // - Interrupts handlers entries are 4 bytes wide (segment + offset)
    if (p->slave) {
        outb(PIC_CMD(p), ICW1_INIT | ICW1_INTERVAL4 | ICW1_ICW4);
    } else {
        outb(PIC_CMD(p), ICW1_INIT | ICW1_INTERVAL4 | ICW1_SINGLE | ICW1_ICW4);
    }
    // Second configuration word (ICW2): offset in interrupts table.
    outb(PIC_DATA(p), p->idt_offset);
    // Third configuration word (ICW3): IRQ where the slave PIC is wired.
    if (p->slave) {
        outb(PIC_DATA(p), 1 << p->slave->irq);
    }
    // Fourth configuration word (ICW4): 8086 mode.
    outb(PIC_DATA(p), ICW4_8086);
    // Mask all interrupts.
    outb(PIC_DATA(p), 0xff);
}

void p8259a_irq_enable(const struct pic *p, int irq) {
    uint8_t mask = 1 << irq;
    uint8_t value;

    value = inb(PIC_DATA(p));
    value &= ~mask;
    outb(PIC_DATA(p), value);
}

void p8259a_irq_disable(const struct pic *p, int irq) {
    uint8_t mask = 1 << irq;
    uint8_t value;

    value = inb(PIC_DATA(p));
    value |= mask;
    outb(PIC_DATA(p), value);
}

void p8259a_irq_ack(const struct pic *p) { outb(PIC_CMD(p), 0x20); }
