// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CF20_H_
#define _CF20_H_

#include <stdint.h>

// Size of a Compact Flash sector.
#define CF20_SECTOR_SIZE 512

// cf20_initialize initialize compact flash devices.
void cf20_initialize(void);

#endif  // _CFI_H_
