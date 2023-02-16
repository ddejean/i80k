// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _MEM_H_
#define _MEM_H_

#include "stdint.h"

// ksegmemcpy copy <n> bytes from <src_seg>:<src> to <dst_seg>:<dst>
extern ksegmemcpy(void *dst, uint16_t dst_seg, void *src, uint16_t src_seg,
                  size_t n);

#endif  // _MEM_H_
