// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <sys/types.h>

// kmemset set the region of memory of size <count> located at <dst> with the
// least significant byte of <value>.
extern void *memset(void *dst, int value, size_t count);

#endif // _STRINGS_H_
