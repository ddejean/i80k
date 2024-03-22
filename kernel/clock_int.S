// Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>
//
// Provides an interrupt handler for the clock.

#include <board.h>

.code16

.section .text

.extern clock_handler

.global clock_int_handler
clock_int_handler:
    // Save the only register we'll use.
    push %ax
    push %bx
    push %cx
    push %dx
    push %ds
    push %es
    // Set kernel data segment.
    mov $KERNEL_DS, %ax
    mov %ax, %ds
    mov %ax, %es
    // Call the interrupt handler.
    call clock_handler
    // Restore the touched register.
    pop %es
    pop %ds
    pop %dx
    pop %cx
    pop %bx
    pop %ax
    // Return from interrupt.
    iret
