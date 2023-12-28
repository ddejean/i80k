// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provides an interrupt handler for the clock.

#include <board.h>

.code16

.section .text

.extern ticks

.global clock_int_handler
clock_int_handler:
    // Save the only register we'll use.
    push %ax
    // Ack the interrupt controller.
    movb $0x20, %al
    outb %al, $0x20
    // Load the current tick value
    mov ticks, %ax
    // Increment the tick least significant word by one and keep the carry.
    clc
    adc $0x1, %ax
    mov %ax, ticks
    // If no carry, do not care of the most significant word.
    jnc _done
    // Carry is set, we need to increment the most significant word.
    mov ticks+2, %ax
    inc %ax
    mov %ax, ticks+2
_done:
    // Restore the touched register.
    pop %ax
    // Return from interrupt.
    iret
