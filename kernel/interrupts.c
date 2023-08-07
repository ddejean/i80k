// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "interrupts.h"

#include <stdint.h>
#include <string.h>

#include "board.h"
#include "cpu.h"

// struct int_desc describes the content of an interrupt descriptor from the
// interrupt descriptors table.
struct int_desc {
    void (*handler)(void);  // Reference to the interrupt routine.
    uint16_t cs;            // Code segment where the routine is located.
};

// Interrupt Descriptor Table start at address 0x00000 with 256 entries.
// The location of the table is handled by the linker script (kernel.lds).
static struct int_desc idt[256] __attribute__((section(".idt")));

// Code segment of the kernel.
static uint16_t code_segment;

void interrupts_setup(uint16_t cs) {
    // Save the kernel code segment.
    code_segment = cs;
    // Clear the IDT.
    memset(idt, 0, sizeof(idt));
}

void interrupts_handle(uint8_t index, void (*handler)(void)) {
    idt[index].handler = handler;
    idt[index].cs = code_segment;
}
