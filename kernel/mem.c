// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "mem.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "devices.h"
#include "fmem.h"

#define MEM_MAGIC0 0x55AA
#define MEM_MAGIC1 0xAA55

// Memory map declared by the board.
static struct memmap *map;

static bool mem_check_segment(uint16_t seg) {
    volatile u16_fptr_t ptr0 = fmem_u16_fptr(seg, (void *)0);
    volatile u16_fptr_t ptr1 = fmem_u16_fptr(seg, (void *)2);

    *ptr0 = MEM_MAGIC0;
    *ptr1 = MEM_MAGIC1;
    return *ptr0 == MEM_MAGIC0 && *ptr1 == MEM_MAGIC1;
}

void mem_initialize(void) {
    const struct device *dev;

    // Get the device that declares the memory map.
    dev = board_get_by_name("memmap");
    if (!dev) {
        printf("mem: memory map not found\n");
        return;
    }

    // The memory map is located in the config.
    map = (struct memmap *)dev->config;

    // Check the memory.
    uint16_t seg = map->ram.segment;
    for (size_t i = 0; i < map->ram.count; i++) {
        if (!mem_check_segment(seg)) {
            printf("mem: bad memory area %04x:0000\n", seg);
        }
        // Move one segment further.
        seg += 0x1000;
    }
}