// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>

#include "uart.h"

int putchar(int c) {
    uart_write((char *)&c, sizeof(char));
    return 1;
}

int puts(const char *s) {
    int i = 0;
    while (*s) {
        putchar(*s);
        s++;
        i++;
    }
    putchar('\n');
    return ++i;
}