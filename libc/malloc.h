// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <stddef.h>

// malloc allocates <size> bytes and returns a pointer to the allocated memory.
// The memory is not initialized. If <size> is 0, then malloc returns either
// NULL or a unique pointer value that can later be successfully passed to
// free().
void *malloc(size_t size);

// free frees the memory space pointed to by <ptr>, which must have been
// returned by a previous call to malloc(). Otherwise or if free(ptr) has
// already been called before, undefined behavior occurs. If <ptr> is NULL, no
// operation is performed.
void free(void *ptr);

#endif  // _MALLOC_H_
