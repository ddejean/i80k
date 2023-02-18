// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STDDEF_H_
#define _STDDEF_H_

#include <sys/types.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define offsetof(st, m) ((size_t) & (((st *)0)->m))

#endif  // _STDDEF_H_
