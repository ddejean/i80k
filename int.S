// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Contains the various of interrupt handlers called by the CPU from the IDT.

.code16

.section .text

.extern uart_handler
.global uart_int_handler
uart_int_handler:
    // Save current activity state.
    push %ax
    push %bx
    push %cx
    push %dx
    push %di
    push %si
    push %ds
    push %es
    // Call the C interrupt handler.
    call uart_handler
    // Restore current activity state.
    pop %es
    pop %ds
    pop %si
    pop %di
    pop %dx
    pop %cx
    pop %bx
    pop %ax

.extern clock_handler
.global clock_int_handler
clock_int_handler:
    // Save current activity state.
    push %ax
    push %bx
    push %cx
    push %dx
    push %di
    push %si
    push %ds
    push %es
    // Call the C interrupt handler.
    call clock_handler
    // Restore current activity state.
    pop %es
    pop %ds
    pop %si
    pop %di
    pop %dx
    pop %cx
    pop %bx
    pop %ax
    iret

