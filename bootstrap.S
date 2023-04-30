/* Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com> */

#include "board.h"

.code16

.section .text

bootstrap:
    // Jump to the begining of the ROM with the appropriate kernel segment.
    jmp $KERNEL_CS, $start
