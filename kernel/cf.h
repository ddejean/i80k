// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CF_H_
#define _CF_H_

#include <stdint.h>

// Size of a Compact Flash sector.
#define CF_SECTOR_SIZE 512

// cf_initialize initialize compact flash devices.
void cf_initialize(void);

// cf_read_blocks reads |count| sectors starting at sector |addr| and puts the
// results in |buffer|. |buffer| is expected to be long enough to contain
// at least count*CF_SECTOR_SIZE bytes.
void cf_read_blocks(uint32_t addr, unsigned int count, char *buffer);

// cf_write_blocks writes |count| sectors starting at sector |addr| reading them
// from |buffer|. The buffer is expected to be long enough to contain at least
// count*CF_SECTOR_SIZE bytes.
void cf_write_blocks(uint32_t addr, unsigned int count, char *buffer);

#endif  // _CFI_H_
