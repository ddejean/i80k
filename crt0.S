// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

// _kernel C symbol has to be declared as coming from another object file.
.extern kernel

// _start is the begining of the ROM, it has to be declared in a special way
// to ensure the linker sees it.
.global start
start:
    // Set the stack segment and pointer at the end of the RAM.
    movw $0, %ax      // Stack segment: first segment where kernel is located.
    movw %ax, %ss
    movw $0, %ax      // Stack: stored at the end of the first segment
    movw %ax, %sp
    movw $0, %ax      // Data: first memory segment.
    movw %ax, %ds
    movw %ax, %es
    // Call the C space
    push %cs          // Provide the code segment to the C world.
    call kernel
    add $2, %sp
    // Should never be reached, but in case hang forever.
    cli
    hlt
