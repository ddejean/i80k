// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "firmware.h"

#include <stdint.h>
#include <string.h>

#include "board.h"
#include "mem.h"

// End of the text segment, virtual address.
extern uint8_t _text_end[];

// Start of the initialized data segment, virtual address. Linker parameter set
// by the build toolchain.
extern uint8_t _data_start[];

// End of initialized data segment, virtual address.
extern uint8_t _data_end[];

// End of uninitialized data segment, virtual address.
extern uint8_t _bss_end[];

void firmware_data_setup(void) {
    size_t data_sz;

    data_sz = _data_end - _data_start;
    // Copy the data section from the kernel segment (eeprom) to the kernel
    // data segment.
    ksegmemcpy(_data_start, KERNEL_DS, _text_end, KERNEL_CS, data_sz);
}

void* firmware_data_end(void) {
    return (void*)_bss_end;
}

void firmware_bss_setup(void) {
    size_t bss_sz = _bss_end - _data_end;
    // .bss section is located between the end of .data (_edata) and the end of
    // the .bss section (_end).
    memset(_data_end, 0, bss_sz);
}
