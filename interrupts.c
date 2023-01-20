// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"
#include "cpu.h"
#include "interrupts.h"

// struct int_desc describes the content of an interrupt descriptor from the
// interrupt descriptors table.
struct int_desc {
    void (*handler)(void);  // Reference to the interrupt routine.
    uint16_t cs;            // Code segment where the routine is located.
};

// Interrupt Descriptor Table, allocated in crt0.S. It must be located at
// address 0000:0000.
extern struct int_desc idt[256];

// Code segment of the kernel.
static uint16_t code_segment;

// Helpers to manipulate the PIC.
static void pic_initialize(uint8_t offset);
static void pic_enable(uint8_t mask);
static void pic_disable(uint8_t mask);

void interrupts_setup(uint16_t cs) {
    // Save the kernel code segment.
    code_segment = cs;
    // Clear the IDT.
    // TODO
    // Configure the PIC for the board configuration and the offset in the
    // interrupt table.
    pic_initialize(IDT_IRQ_OFFSET);
}

void interrupts_handle(uint8_t index, void (*handler)(void)) {
    idt[index].handler = handler;
    idt[index].cs = code_segment;
}

void irq_enable(uint8_t mask) {
    pic_enable(mask);
}

void irq_disable(uint8_t mask) {
    pic_disable(mask);
}

// PIC ports
#define PIC_A0_0        PORT_PIC        // PIC with A0=0
#define PIC_A0_1        (PORT_PIC | 1)  // PIC with A0=1

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

static void pic_initialize(uint8_t offset) {
    // First configuration word (ICW1):
    // - Level triggered (not edge triggered)
    // - Interrupts handlers entries are 4 bytes wide (segment + offset)
    // - Single PIC
    outb(PIC_A0_0, ICW1_INIT | ICW1_LEVEL | ICW1_INTERVAL4 | ICW1_SINGLE | ICW1_ICW4);
    // Second configuration word (ICW2): offset in interrupts table. 8 is the
    // classical offset for IBM PC/XT.
    outb(PIC_A0_1, offset);
    // Fourth configuration word (ICW4): 8086 mode.
    outb(PIC_A0_1, ICW4_8086);
    // Mask all interrupts.
    outb(PIC_A0_1, MASK_ALL);
}

static void pic_enable(uint8_t mask) {
    uint8_t value = inb(PIC_A0_1);
    value &= ~mask;
    outb(PIC_A0_1, value);
}

static void pic_disable(uint8_t mask) {
    uint8_t value = inb(PIC_A0_1);
    value |= mask;
    outb(PIC_A0_1, value);
}
