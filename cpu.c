// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "stdint.h"

void outb(uint16_t port, uint8_t data) {
#asm
    push    bp
    mov     bp, sp
    push    dx
    push    ax
    mov     dx, [bp+4]
    mov     al, [bp+6]
    out     dx, al
    pop     ax
    pop     dx
    pop     bp
#endasm
}

uint8_t inb(uint16_t port) {
#asm
    push    bp
    mov     bp, sp
    push    dx
    xor     ax, ax
    mov     dx, [bp+4]
    in      al, dx
    pop     dx
    pop     bp
#endasm
}

void cli(void) {
#asm
    cli
#endasm
}

void sti(void) {
#asm
    sti
#endasm
}

void hlt(void) {
#asm
    hlt
#endasm
}
