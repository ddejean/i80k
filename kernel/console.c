// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>
#include <string.h>

#include "uart.h"

int console_putchar(int c) {
    if ((char)c == '\n') {
        uart_putchar('\r');
    }
    return uart_putchar((char)c);
}

int console_puts(const char *s) {
    int len = 0;
    const char br[2] = {'\r', '\n'};

    len = strlen(s);
    uart_write(s, len);
    uart_write(br, sizeof(br));
    return len + sizeof(br);
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