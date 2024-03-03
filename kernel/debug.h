// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdarg.h>
#include <stdint.h>

#include "board.h"
#include "cpu.h"

#define DEBUG(value)                        \
    do {                                    \
        int _i;                             \
        outb(PORT_DEBUG, (uint8_t)(value)); \
        for (_i = 0; _i < 32767; _i++)      \
            ;                               \
    } while (0)

// debug_dump dumps the |buffer| array to the console in both hex and ascii
// form.
void debug_dump(char *buffer, unsigned int sz);

#endif  // _DEBUG_H_
