// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <string.h>
#include <sys/signal.h>
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

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
    int ret;
    __asm__ __volatile__(
        "mov $0xf7, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "mov %4, %%si\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(idtype), "g"(id), "g"(infop), "g"(options)
        : "ax", "bx", "cx", "dx", "si");
    return ret;
}

pid_t waitpid(pid_t pid, int *wstatus, int options) {
    int ret;
    siginfo_t info;

    memset(&info, 0, sizeof(info));
    ret = waitid(P_PID, pid, &info, options);
    if (ret < 0) {
        return ret;
    }
    if (wstatus) {
        wstatus = 0;
    }
    return info.si_pid;
}

pid_t wait(int *wstatus) { return waitpid(-1, wstatus, 0); }
