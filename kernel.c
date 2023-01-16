// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "interrupts.h"

void kernel() {
    interrupts_setup();
    cli();
    hlt();
}
