// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "hwalloc.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

#define PAGES_HOLDERS 4
#define PAGES_COUNT 16
#define PAGES_FULL 0xffff

#define HOLDER_SHIFT 14
#define PAGE_SHIFT 10

struct seg {
    uint16_t segment;
    uint16_t holders[PAGES_HOLDERS];
};

// Number of managed segments.
unsigned int segments_count;
// Array of the segments managed by this allocator.
struct seg *segments;

void hw_alloc_init(unsigned int first_seg, unsigned int last_seg) {
    size_t sz;
    unsigned int i;

    // Compute the number of segments available. Don't forget that the last
    // segment is included, that's why we need to add +1.
    segments_count = last_seg - first_seg + 1;
    sz = segments_count * sizeof(struct seg);
    segments = malloc(sz);
    memset(segments, 0, sz);
    for (i = first_seg; i <= last_seg; i++) {
        segments[i - first_seg].segment = i << 12;
    }
}

struct hw_page *hw_page_alloc() {
    unsigned int i, j, k;
    uint16_t pages;
    struct hw_page *page;

    for (i = 0; i < segments_count; i++) {
        for (j = 0; j < PAGES_HOLDERS; j++) {
            pages = segments[i].holders[j];
            // Skip page holders that are already full.
            if (pages == PAGES_FULL) {
                continue;
            }
            // Find the first available page.
            for (k = 0; k < PAGES_COUNT; k++) {
                // Skip used page.
                if (pages & (1 << k)) {
                    continue;
                }
                // Mark the page as used.
                segments[i].holders[j] |= (1 << k);
                // Return a descriptor to the caller.
                page = malloc(sizeof(*page));
                page->index = i;
                page->holder = j;
                page->page = k;
                return page;
            }
        }
    }
    return NULL;
}

uint16_t hw_page_seg(struct hw_page *p) { return segments[p->index].segment; }

uint16_t *hw_page_addr(struct hw_page *p) {
    return (uint16_t *)((p->holder << HOLDER_SHIFT) | (p->page << PAGE_SHIFT));
}

void hw_page_free(struct hw_page *p) {
    // Mark the page as free.
    segments[p->index].holders[p->holder] &= ~(1 << p->page);
    // Free the descriptor.
    free(p);
}
