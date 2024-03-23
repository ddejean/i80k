// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _ASSERT_H_
#define _ASSERT_H_

#define assert(__e)                            \
    do {                                       \
        if (!(__e)) {                          \
            __asm__ __volatile__("int $3" ::); \
        }                                      \
    } while (0)

#endif  // _ASSERT_H_
