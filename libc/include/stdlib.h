// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>

extern void *malloc(size_t size);
extern void free(void *ptr);

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))

#define ALIGN(a, b) ROUNDUP(a, b)
#define IS_ALIGNED(a, b) (!(((uintptr_t)(a)) & (((uintptr_t)(b)) - 1)))

#endif  // _STDLIB_H_
