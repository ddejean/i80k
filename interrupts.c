// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "cpu.h"
#include "interrupts.h"

// PIC ports
#define PIC_CMD         0x0
#define PIC_DATA        0x1

// PIC command words
#define ICW1_ICW4	    0x01		// ICW4 (not) needed
#define ICW1_SINGLE	    0x02		// Single (cascade) mode
#define ICW1_INTERVAL4	0x04		// Call address interval 4 (8)
#define ICW1_LEVEL	    0x08		// Level triggered (edge) mode
#define ICW1_INIT	    0x10		// Initialization bit

#define ICW4_8086	    0x01		// 8086/88 (MCS-80/85) mode
#define ICW4_AUTO	    0x02		// Auto (normal) EOI
#define ICW4_BUF_SLAVE	0x08		// Buffered mode/slave
#define ICW4_BUF_MASTER	0x0C		// Buffered mode/master
#define ICW4_SFNM	    0x10		// Special fully nested (not)

void interrupts_setup(void) {
    // First configuration word (ICW1):
    // - Level triggered (not edge triggered)
    // - Interrupts handlers entries are 4 bytes wide (segment + offset)
    // - Single PIC
    outb(PIC_CMD, ICW1_INIT | ICW1_LEVEL | ICW1_INTERVAL4 | ICW1_SINGLE | ICW1_ICW4);
    // Second configuration word (ICW2): offset in interrupts table.
    outb(PIC_DATA, 0);
    // Fourth configuration word (ICW4): 8086 mode.
    outb(PIC_DATA, ICW4_8086);
    // Mask all interrupts.
    outb(PIC_DATA, INT_ALL);
    // Enable CPU interrupts.
    sti();
}

void interrupts_enable(uint8_t mask) {
    uint8_t value = inb(PIC_DATA);
    value &= ~mask;
    outb(PIC_DATA, value);
}

void interrupts_disable(uint8_t mask) {
    uint8_t value = inb(PIC_DATA);
    value |= mask;
    outb(PIC_DATA, value);
}
