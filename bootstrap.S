; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

bits 16
org 0xFFF0

_bootstrap:
    ; The linker is not able to generate a file locate at address 0x8000,
    ; as a workaround use segment 0xF800 instead of 0xF000.
    jmp 0xf000:0x8000
