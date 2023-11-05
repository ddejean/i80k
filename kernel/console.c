// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>
#include <string.h>

#include "board.h"

#ifdef BOARD_8088_REV2
#include "p8251.h"
#define uart_initialize p8251_initialize
#define uart_putchar p8251_putchar
#define uart_write p8251_write
#define uart_read p8251_read
#endif

#ifdef BOARD_8088_REV3
#include "pc16550.h"
#define uart_initialize pc16550_initialize
#define uart_putchar pc16550_putchar
#define uart_write pc16550_write
#define uart_read pc16550_read
#endif

void console_initialize(void) { uart_initialize(19200); }

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