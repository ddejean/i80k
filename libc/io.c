// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

int putchar(int c) {
    __asm__ __volatile__(
        "mov $0x02, %%ah\n"
        "int $0x21"
        :  // no output
        : "d"(c)
        : "ah");
    return 1;
}

int puts(const char* s) {
    __asm__ __volatile__(
        "mov $0x09, %%ah\n"
        "int $0x21"
        :  // no output
        : "d"(s)
        : "ah");
    return 1;
}

int getchar(void) {
    int c;
    __asm__ __volatile__(
        "mov $0x01, %%ah\n"
        "int $0x21\n"
        "mov %%ax, %0\n"
        : "=r"(c)
        :  // no input
        : "ax");
    return c;
}
