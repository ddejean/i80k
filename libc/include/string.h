// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <stddef.h>

// memset set the region of memory of size <count> located at <dst> with the
// least significant byte of <value>.
extern void *memset(void *dst, int value, size_t count);

// memcpy copies <count> bytes from <src> to <dst>.
extern void *memcpy(void *dst, void *src, size_t count);

// strnlen returns the number of chars pointed by s (final '\0' excluded).
extern size_t strlen(const char *s);

// strnlen returns the number of chars pointed by s (final '\0' excluded) within
// 0 and <maxlen>.
extern size_t strnlen(const char *s, size_t maxlen);

// strcmp compares the two strings <s1> and <s2>. Returns and integer indicating
// the result of the comparison.
extern int strcmp(const char *s1, const char *s2);

#endif  // _STRINGS_H_
