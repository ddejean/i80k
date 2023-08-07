// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Buddy memory allocator based on brk()/sbrk() to obtain heap memory. The
// maximum allocation size is limited to 4K.

#include "malloc.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel.h"

// Heap, NULL at first and will grow with the allocations.
char *heap;

// Total heap size.
int heap_size;

// Free block header.
struct free_block {
    struct free_block *next;
};

// Allocated block header.
struct block_header {
    int size;
    char mem[];
};

#define MAX_INDEX 12
// Maximum allocable size at once.
#define PAGE_SIZE (1 << MAX_INDEX)
// Minimum memory block size.
#define MIN_SIZE 8

// Array of free blocks.
static struct free_block fb_table[MAX_INDEX + 1];

// getindex returns the index of a block is size <size>.
static int getindex(size_t size) {
    static int min_index;
    int index;
    size_t block_size, sz;

    // Compute the minimum size only once.
    min_index = 0;
    if (min_index == 0) {
        sz = 1;
        while (sz < MIN_SIZE) {
            sz = sz << 1;
            min_index++;
        }
    }

    index = min_index;
    block_size = MIN_SIZE;
    while (block_size < size) {
        block_size = block_size << 1;
        index++;
    }
    return index;
}

// getbuddy returns the address of the buddy block described by <index> and
// <addr>.
static void *getbuddy(int index, void *addr) {
    return heap + ((size_t)(1 << index) ^ ((size_t)addr - (size_t)heap));
}

// fb_add adds a free bloc of size 2^<index> starting at address <addr>
static void fb_add(int index, void *address) {
    ((struct free_block *)address)->next = fb_table[index].next;
    fb_table[index].next = address;
}

// fb_pop removes a block of size 2^<index>.
static void *fb_pop(int index) {
    struct free_block *block = fb_table[index].next;
    if (block != NULL) fb_table[index].next = block->next;
    return block;
}

// fb_remove removes a free block at <index> with address <addr>.
static int fb_remove(int index, void *address) {
    struct free_block *block = &(fb_table[index]);
    while (block->next != NULL && block->next != address) block = block->next;
    if (block->next == address) {
        block->next = block->next->next;
        return 1;
    } else {
        return 0;
    }
}

void *malloc(size_t size) {
    int index, i;
    struct block_header *header;
    size_t full_size;
    void *zone;

    if (!size) {
        return NULL;
    }

    // Initialize the heap if this is the first time.
    if (!heap) {
        heap = sbrk(0);
    }

    full_size = size + sizeof(struct block_header);
    index = getindex(full_size);
    i = index;
    if (index > MAX_INDEX) return NULL;

    // Get the smallest free bloc.
    zone = fb_pop(i);
    while (zone == NULL && i < MAX_INDEX) {
        zone = fb_pop(++i);
    }

    // No more memory available, get some.
    if (zone == NULL) {
        if (sbrk(PAGE_SIZE) == (void *)-1) {
            // No more memory available at all.
            return NULL;
        }
        zone = heap + heap_size;
        heap_size += PAGE_SIZE;
        i = MAX_INDEX;
    }

    // Divide it by two until we reach the correct size.
    while (i-- > index) {
        fb_add(i, getbuddy(i, zone));
    }

    // Prepare the header.
    header = zone;
    header->size = full_size;
    return (void *)header->mem;
}

void free(void *ptr) {
    int i;
    void *zone, *buddy;
    struct block_header *header;

    if (!ptr) {
        return;
    }

    zone = (char *)ptr - offsetof(struct block_header, mem);
    header = (struct block_header *)zone;
    i = getindex(header->size);
    buddy = getbuddy(i, zone);

    // Obtain the free buddies while there is some.
    while (i < MAX_INDEX && fb_remove(i, buddy)) {
        zone = (zone <= buddy) ? zone : buddy;
        buddy = getbuddy(++i, zone);
    }

    // Insert the big free block.
    fb_add(i, zone);
}
