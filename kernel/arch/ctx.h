// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CTX_H_
#define _CTX_H_

#include <stdint.h>

struct context {
    // Stack pointer.
    void *sp;
    // Stack segment.
    uint16_t ss;
};

void ctx_switch(struct context *prev, struct context *next);

#endif  // _CTX_H_
