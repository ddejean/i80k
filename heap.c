// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "heap.h"

#include <stddef.h>
#include <stdint.h>

#include "debug.h"

// Base address of the kernel heap.
void *heap_base;

// Maximum address of the kernel heap.
void *heap_max;

// Current heap address.
void *heap;

void heap_initialize(void *start, void *end) {
    printk("Heap: [%p:%p], %u bytes\r\n", start, end, end - start);

    heap_base = start;
    heap_max = end;
    heap = heap_base;
}

int brk(void *addr) {
    if (addr < heap_base || addr > heap_max) {
        // TODO: set errno.
        return -1;
    }
    heap = addr;
    return 0;
}

void *sbrk(intptr_t increment) {
    void *old_heap, *new_heap;

    if (!increment) {
        return heap;
    }

    new_heap = (char *)heap + increment;
    if (new_heap < heap_base || new_heap > heap_max) {
        // TODO: set errno.
        return (void *)-1;
    }
    old_heap = heap;
    heap = new_heap;
    return old_heap;
}
