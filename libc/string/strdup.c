// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdlib.h>
#include <string.h>

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}
