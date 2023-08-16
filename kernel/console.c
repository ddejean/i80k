// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>
#include <string.h>

#include "uart.h"

int console_putchar(int c) {
    uart_write((char *)&c, sizeof(char));
    return 1;
}

int console_puts(const char *s) {
    size_t len;
    char c = '\n';

    len = strlen(s);
    uart_write(s, len);
    uart_write(&c, sizeof(c));
    return ++len;
}

int console_getchar(void) {
    char c;
    int len;

    len = uart_read(&c, sizeof(c));
    if (len == 1) {
        return c;
    }
    return -1;
}