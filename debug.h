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

// printk puts on message on the kernel console.
int printk(const char* format, ...);

#endif  // _DEBUG_H_
