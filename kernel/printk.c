// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdarg.h>
#include <stdio.h>

#include "uart.h"

#define PRINTK_BUZ_SIZE 1024

// Buffer that will handler the string to print.
char buffer[PRINTK_BUZ_SIZE];

int printk(const char* format, ...) {
    va_list args;
    int i;

    va_start(args, format);
    i = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return uart_write(buffer, i);
}
