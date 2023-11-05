// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>
#include <string.h>

#include "p8251.h"

void console_initialize(void) { p8251_initialize(19200); }

int console_putchar(int c) {
    if ((char)c == '\n') {
        p8251_putchar('\r');
    }
    return p8251_putchar((char)c);
}

int console_puts(const char *s) {
    int len = 0;
    const char br[2] = {'\r', '\n'};

    len = strlen(s);
    p8251_write(s, len);
    p8251_write(br, sizeof(br));
    return len + sizeof(br);
}

int console_getchar(void) {
    char c;
    int len;

    len = p8251_read(&c, sizeof(c));
    if (len == 1) {
        return c;
    }
    return -1;
}