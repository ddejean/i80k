// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <sys/timespec.h>
#include <sys/types.h>
#include <time.h>

int clock_gettime(clockid_t clockid, struct timespec *tp) {
    int ret;
    __asm__ __volatile__(
        "mov $0xe3, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "rm"(clockid), "rm"(tp)
        : "ax", "bx", "cx");
    return ret;
}

int clock_nanosleep(clockid_t clockid, int flags,
                    const struct timespec *request, struct timespec *remain) {
    int ret;
    __asm__ __volatile__(
        "mov $0xe6, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "mov %4, %%si\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "rm"(clockid), "rm"(flags), "rm"(request), "rm"(remain)
        : "ax", "bx", "cx", "dx", "si");
    return ret;
}
