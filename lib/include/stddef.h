// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STDDEF_H_
#define _STDDEF_H_

#define NULL ((void*)0)
#define offsetof(TYPE, MEMBER) __builtin_offsetof(TYPE, MEMBER)

typedef unsigned int size_t;
typedef int ptrdiff_t;

#endif  // _STDDEF_H_
