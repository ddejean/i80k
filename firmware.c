// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "firmware.h"

#include "board.h"
#include "stdint.h"

// End of the text segment, virtual address.
extern uint8_t _etext[];

// Start of the initialized data segment, virtual address. Linker parameter set
// by the build toolchain.
#define _sdata 0x400

// End of initialized data segment, virtual address.
extern uint8_t _edata[];

// End of uninitialized data segment, virtual address.
extern uint8_t _end[];

// Offset of the start of the initialized data segment within the executable
// file in blocks of 16 bytes.
extern uint8_t _segoff[];

void firmware_data_setup(void) {
    uint16_t rom_data_start;
    size_t data_sz;

    // The content of the data section is located of the code in the ROM. To
    // find the location, use the segment offset defined by ld86.
    rom_data_start = (uint16_t)_segoff * 16 + _sdata;
    data_sz = _edata - _sdata;
    // Copy the data section from the kernel segment (eeprom) to the kernel
    // data segment.
    ksegmemcpy(_sdata, KERNEL_DS, rom_data_start, KERNEL_CS, data_sz);
}

void firmware_bss_setup(void) {
    size_t bss_sz = (uint16_t)_end - (uint16_t)_edata;
    // .bss section is located between the end of .data (_edata) and the end of
    // the .bss section (_end).
    kmemset(_edata, 0, bss_sz);
}
