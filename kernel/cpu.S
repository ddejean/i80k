// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

.global reboot
reboot:
    // Clear the flags register.
    push $0
    popf
    // Set segments to 0x0
    xor %ax, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    // Jump to the boot address FFFF:0000
    jmp $0xffff, $0x0000