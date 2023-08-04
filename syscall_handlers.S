// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Syscall handlers implementation

.code16

.section .text

.global int21_handler
int21_handler:
    iret
