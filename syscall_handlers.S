// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Syscall handlers implementation

.code16

.section .text

.extern syscall_int21
.global int21_handler
int21_handler:
    push %dx
    push %ds
    push %ax
    call syscall_int21
    iret
