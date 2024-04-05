// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Contains the UART interrupt handler.

#include <board.h>

.code16

.section .text

.extern cf20_handler
.global cf20_int_handler
cf20_int_handler:
    // Save current activity state.
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
    // Note: bp, si and di are saved by the callee anyway. When multiple segment
    // will be used, we'll need to save ds, es and then set them to the kernel
    // values.  Call the C interrupt handler.
    call cf20_handler
    // Restore current activity state.
    pop %es
    pop %ds
    pop %dx
    pop %cx
    pop %bx
    pop %ax
    iret
