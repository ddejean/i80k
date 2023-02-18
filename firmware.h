// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

// firmware_data_setup relocates the data section of the firmware to ensure
// initialized data will have the appropriate values.
void firmware_data_setup(void);

// firmware_data_end returns the end of the data section which is also the start
// of the heap.
void* firmware_data_end(void);

// firmware_bss_setup prepares the bss section to ensure all uninitialized data
// in it are set to 0.
void firmware_bss_setup(void);

#endif  // _FIRMWARE_H_
