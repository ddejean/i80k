// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CFI_H_
#define _CFI_H_

#include <stdint.h>

// cfi_initialize detects any present flash devices and prepares it for use.
void cfi_initialize(void);

// cfi_chip_erase erases the content of the flash chip.
void cfi_chip_erase(void);

// cfi_write programs |bytes| at |offset| from the begining of the flash device.
void cfi_write(uint32_t offset, uint8_t byte);

#endif  // _CFI_H_
