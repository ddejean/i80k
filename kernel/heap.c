// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "heap.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Base address of the kernel heap.
void *heap_base;

// Maximum address of the kernel heap.
void *heap_max;

// Current heap address.
void *heap;

void heap_initialize(void *start, void *end) {
    printf("Heap: [%p:%p], %u bytes\n", start, end, end - start);

    heap_base = start;
    heap_max = end;
    heap = heap_base;
}

void *heap_brk(void *addr) {
    if (addr == (void *)0) {
        return heap;
    }
    if (addr < heap_base || addr > heap_max) {
        // TODO: set errno.
        return (void *)-1;
    }
    heap = addr;
    return heap;
}
