// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

__attribute__((__noreturn__)) void exit(int status) {
    __asm__ __volatile__(
        "mov $0x3c, %%ax\n"
        "mov %0, %%bx\n"
        "int $0x80\n"
        :  // no output
        : "r"(status)
        : "ax", "bx");

    // No return function should never return, satisfy the compiler.
    while (1)
        ;
}

pid_t getpid() {
    pid_t pid;
    __asm__ __volatile__(
        "mov $0x27, %%ax\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(pid)
        :  // no input
        : "ax");
    return pid;
}

pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage) {
    int ret;
    __asm__ __volatile__(
        "mov $0x3d, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "mov %4, %%si\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(pid), "g"(wstatus), "g"(options), "g"(rusage)
        : "ax", "bx", "cx", "dx", "si");
    return ret;
}

pid_t wait3(int *wstatus, int options, struct rusage *rusage) {
    return wait4(-1, wstatus, options, rusage);
}

pid_t waitpid(pid_t pid, int *wstatus, int options) {
    return wait4(pid, wstatus, options, NULL);
}

pid_t wait(int *wstatus) { return wait4(-1, wstatus, 0, NULL); }
