// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

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
