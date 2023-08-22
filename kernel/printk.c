// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdarg.h>
#include <stdio.h>

#include "uart.h"

int printk(const char* format, ...) {
    va_list args;
    int i;

    va_start(args, format);
    i = vprintf(format, args);
    va_end(args);
    return i;
}
