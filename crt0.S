// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

.code16

.section .text

// _kernel C symbol has to be declared as coming from another object file.
.extern kernel

.global start
start:
    // Set the stack segment and pointer at the end of the RAM.
    mov $KERNEL_SS, %ax     // Stack segment: first memory segment.
    mov %ax, %ss
    xor %ax, %ax            // Stack: stored at the end of the first segment
    mov %ax, %sp

    // Relocate the .text section to the future kernel section in RAM.
    call text_copy

    // Relocate the .data section to the future kernel section in RAM.
    call data_copy

    // Clear the .bss section before calling the C world.
    call bss_clear

    // Jump to the new kernel location.
    jmp $KERNEL_CS, $_1
_1:

    mov $KERNEL_DS, %ax     // Data: first memory segment.
    mov %ax, %ds
    mov %ax, %es

    // Call the C space
    call kernel

    // Should never be reached, but in case hang forever.
    cli
    hlt

// data_copy copies the .text section of the kernel to the future location of
// the kernel in RAM.
text_copy:
    mov $BOOTSTRAP_CS, %ax
    mov %ax, %ds
    mov $KERNEL_CS, %ax
    mov %ax, %es
    mov $_text_start, %ax
    mov %ax, %si
    mov %ax, %di
    mov $_text_end, %cx
    sub %ax, %cx
    cld
    rep movsb
    ret

// data_copy copies the .data section of the kernel to the future location of
// the kernel in RAM.
data_copy:
    mov $BOOTSTRAP_CS, %ax
    mov %ax, %ds
    mov $KERNEL_DS, %ax
    mov %ax, %es
    mov $_data_start, %ax
    mov %ax, %si
    mov %ax, %di
    mov $_data_end, %cx
    sub %ax, %cx
    cld
    rep movsb
    ret

// bss_clear prepare the .bss section of the kernel prior moving to the C world.
bss_clear:
    // Destination: KERNEL_DS:_bss_start
    mov $KERNEL_DS, %ax
    mov %ax, %es
    mov $_bss_start, %ax
    mov %ax, %di
    // Length: _bss_end - _bss_start
    mov $_bss_end, %cx
    sub %ax, %cx
    // Value: 0
    xor %ax, %ax
    // Trigger the set.
    cld
    rep stosb
    ret
