// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _PAGE_H_
#define _PAGE_H_

#include "fmem.h"

struct page {
    void_fptr_t addr;
    size_t order;
};

void page_initialize(void);

// page_add_area adds |area| to the page allocator for future use.
void page_add(void_fptr_t area, size_t order);

// page_alloc returns a continous set of 2^|order| pages.
struct page *page_alloc(size_t order);

// page_free frees the set of pages referenced by |p|.
void page_free(struct page *p);

#endif  // _PAGE_H_
