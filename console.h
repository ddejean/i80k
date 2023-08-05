// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

// console_putchar() writes <c> onto the binded console.
int console_putchar(int c);

// console_puts() writes <s> onto the binded console and add a carriage return.
int console_puts(const char *s);

#endif  // _CONSOLE_H_
