// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <stdint.h>

#ifndef _FMEM_H_
#define _FMEM_H_

#ifdef __IA16_ARCH_ANY
#define far __far
#else
#define far
#endif

// Far pointer type helpers
typedef void far* void_fptr_t;
typedef uint8_t far* u8_fptr_t;
typedef uint16_t far* u16_fptr_t;

inline void_fptr_t fmem_void_fptr(uint16_t seg, void* ptr) {
    return (void far*)((((uint32_t)seg) << 16) | (uint32_t)ptr);
}

inline u16_fptr_t fmem_u16_fptr(uint16_t seg, uint16_t* ptr) {
    return (u16_fptr_t)((((uint32_t)seg) << 16) | (uint32_t)ptr);
}

// fmemcpy is a memcpy implementation using far pointers.
void fmemcpy(void far* dst, void far* src, size_t n);

#endif  //_FMEM_H_
