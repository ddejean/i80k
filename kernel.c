// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "cpu.h"

void kernel() {
    // Put something on the debug port.
    outb(0x2000, 1);
    cli();
    hlt();
}
