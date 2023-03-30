/* Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com> */

.code16

.section .text

bootstrap:
    // Jump to the begining of the ROM with the appropriate kernel segment.
    jmp $0xf000, $start
