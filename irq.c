// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "irq.h"

#include <stdint.h>

#include "board.h"
#include "cpu.h"
#include "irq.h"

// PIC ports
#define PIC_A0_0 PORT_PIC        // PIC with A0=0
#define PIC_A0_1 (PORT_PIC | 1)  // PIC with A0=1

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

static inline void pic_initialize(uint8_t offset) {
    // First configuration word (ICW1):
    // - Level triggered (not edge triggered)
    // - Interrupts handlers entries are 4 bytes wide (segment + offset)
    // - Single PIC
    outb(PIC_A0_0, ICW1_INIT | ICW1_INTERVAL4 | ICW1_SINGLE | ICW1_ICW4);
    // Second configuration word (ICW2): offset in interrupts table. 8 is the
    // classical offset for IBM PC/XT.
    outb(PIC_A0_1, offset);
    // Fourth configuration word (ICW4): 8086 mode.
    outb(PIC_A0_1, ICW4_8086);
    // Mask all interrupts.
    outb(PIC_A0_1, MASK_ALL);
}

static inline void pic_enable(uint8_t mask) {
    uint8_t value = inb(PIC_A0_1);
    value &= ~mask;
    outb(PIC_A0_1, value);
}

static inline void pic_disable(uint8_t mask) {
    uint8_t value = inb(PIC_A0_1);
    value |= mask;
    outb(PIC_A0_1, value);
}

static inline void pic_ack(void) { outb(PORT_PIC, 0x20); }

void irq_setup(void) {
    // Configure the PIC for the board configuration and the offset in the
    // interrupt table.
    pic_initialize(IDT_IRQ_OFFSET);
}

void irq_enable(uint8_t mask) { pic_enable(mask); }

void irq_disable(uint8_t mask) { pic_disable(mask); }

void irq_ack(void) { pic_ack(); }
