// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <unistd.h>

char *_lib_brk;

void *_brk(void *addr) {
    void *ret;
    __asm__ __volatile__(
        "mov $0x0c, %%ax\n"
        "mov %1, %%bx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "r"(addr)
        : "ax", "bx");
    return ret;
}

int brk(void *addr) {
    char *new_brk = _brk(addr);
    if (new_brk != addr) return -1;
    _lib_brk = new_brk;
    return 0;
}

void *sbrk(ptrdiff_t increment) {
    char *start;
    char *end;
    char *new_brk;

    if (!_lib_brk) {
        _lib_brk = _brk((void *)0);
    }

    start = _lib_brk;
    end = start + increment;
    new_brk = _brk(end);
    if (new_brk == (void *)-1) {
        return new_brk;
    } else if (new_brk < end) {
        return (void *)-1;
    }
    _lib_brk = new_brk;
    return start;
}