// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>

#ifndef _FMEM_H_
#define _FMEM_H_

// Far pointer type helpers
typedef uint8_t __far* u8_fptr_t;

inline void __far* fmem_void_fptr(uint16_t seg, void* ptr) {
    return (void __far*)((((uint32_t)seg) << 16) | (uint32_t)ptr);
}

// fmemcpy is a memcpy implementation using far pointers.
void fmemcpy(void __far* dst, void __far* src, size_t n);

#endif  //_FMEM_H_
