// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

static int _open(const char *pathname, int flags, int mode) {
    int ret;
    __asm__ __volatile__(
        "mov $0x02, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(pathname), "g"(flags), "g"(mode)
        : "ax", "bx", "cx", "dx");
    return ret;
}

int open(const char *pathname, int flags, ...) {
    va_list ap;
    int ret;

    va_start(ap, flags);
    ret = _open(pathname, flags, va_arg(ap, int));
    va_end(ap);

    return ret;
}

ssize_t read(int fd, void *buf, size_t count) {
    ssize_t ret;
    __asm__ __volatile__(
        "mov $0x00, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(fd), "g"(buf), "g"(count)
        : "ax", "bx", "cx", "dx");
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    ssize_t ret;
    __asm__ __volatile__(
        "mov $0x01, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(fd), "g"(buf), "g"(count)
        : "ax", "bx", "cx", "dx");
    return ret;
}

int close(int fd) {
    int ret;
    __asm__ __volatile__(
        "mov $0x03, %%ax\n"
        "mov %1, %%bx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(fd)
        : "ax", "bx");
    return ret;
}

int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data) {
    int ret;
    (void)mountflags;
    (void)data;
    __asm__ __volatile__(
        "mov $0xa5, %%ax\n"
        "mov %1, %%bx\n"
        "mov %2, %%cx\n"
        "mov %3, %%dx\n"
        "int $0x80\n"
        "mov %%ax, %0\n"
        : "=r"(ret)
        : "g"(source), "g"(target), "g"(filesystemtype)
        : "ax", "bx", "cx", "dx");
    return ret;
}