// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Syscall handlers implementation

.code16

.section .text

.extern syscall_int21
.global int21_handler
int21_handler:
    push %dx
    push %ax
    call syscall_int21
    add $4, %sp
    iret

.extern syscall_int80
.global int80_handler
int80_handler:
    push %bx
    push %ax
    call syscall_int80
    add $4, %sp
    iret
