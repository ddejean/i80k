// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _HEAP_H_
#define _HEAP_H_

#include <stdint.h>

// heap_initialize prepares the heap for future allocations.
void heap_initialize(void *start, void *end);

// heap_brk
void *heap_brk(void *addr);

#endif  // _HEAP_H_
