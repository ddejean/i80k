// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com

#include "debug.h"

#include <stdio.h>

static char printable(char c) {
    if (c >= 0x20 && c < 0x7f) {
        return c;
    }
    return '.';
}

void debug_dump(char *buffer, unsigned int sz) {
    for (unsigned int i = 0; i < sz; i += 16) {
        printf("%04x | ", i);
        for (unsigned int j = 0; j < 16; j++) {
            if (i + j < sz) {
                printf("%02x ", (unsigned char)buffer[i + j]);
            } else {
                printf("   ");
            }
        }
        printf("| ");
        for (unsigned int j = 0; j < 16; j++) {
            if (i + j < sz) {
                printf("%c", printable(buffer[i + j]));
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}
