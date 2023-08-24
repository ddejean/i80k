// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>
#include <stddef.h>

int sprintf(char* buffer, const char* format, ...);
int snprintf(char* buffer, size_t count, const char* format, ...);
int vsnprintf(char* buffer, size_t count, const char* format, va_list va);
int vprintf(const char* format, va_list va);
int fctprintf(void (*out)(char character, void* arg), void* arg,
              const char* format, ...);
int printf(const char* format, ...);
int putchar(int c);
int puts(const char* s);
int getchar(void);

#endif  // _STDIO_H_
