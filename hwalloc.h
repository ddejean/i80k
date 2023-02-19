// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _HWALLOC_H_
#define _HWALLOC_H_

#include <stdint.h>

#define PAGE_SZ 1024

// hw_area describes an area of hardware memory.
struct hw_page {
    unsigned int index;
    unsigned int holder;
    unsigned int page;
};

// hw_mem_init initializes the hardware memory allocator to manager the zone
// between <first_seg> and <last_seg> included.
extern void hw_alloc_init(unsigned int first_seg, unsigned int last_seg);

// hw_page_alloc allocate a free hardware page.
extern struct hw_page *hw_page_alloc();

// hw_page_segment returns the segment register value for the page <p>.
extern uint16_t hw_page_seg(struct hw_page *p);

// hw_page_segment returns the address value for the page <p>.
extern uint16_t *hw_page_addr(struct hw_page *p);

// hw_page_free release the hardware page <p>.
extern void hw_page_free(struct hw_page *p);

#endif  // _HWALLOC_H_
