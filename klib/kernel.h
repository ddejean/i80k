// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Set of functions provided by the kernel itself to support the implementation
// of the C library.

#include <stdint.h>

// brk sets the end of the data segment to the value specified by addr.
// On success brk() returns 0. On error -1 is returned.
extern int brk(void *addr);

// sbrk() increments the program's data space by increment bytes. Calling sbrk()
// with an increment of 0 can be used to find the current location of the
// program break.
// On success, sbrk() returns the previous program break.  (If the break was
// increased, then this value is a pointer to the start of the newly allocated
// memory).  On error, (void *) -1 is returned.
extern void *sbrk(intptr_t increment);

// putchar() writes a char on the binded console.
extern int putchar(int c);
