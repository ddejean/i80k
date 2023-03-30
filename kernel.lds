/* Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com> */

MEMORY {
  rom       (rx)  : ORIGIN = 0x8000, LENGTH = 0x7FF0
  bootstrap (rx)  : ORIGIN = 0xFFF0, LENGTH = 0x0010
  ram       (rwx) : ORIGIN = 0x0400, LENGTH = 0xFC00
}

SECTIONS {
    .text ORIGIN(rom): {
        *crt0.o(.text*);
        EXCLUDE_FILE(*crt0.o *bootstrap.o) *(.text*);
        _text_end = .;
    } > rom

    .data ORIGIN(ram): {
        _data_start = .;
        *(.rodata*);
        *(.data*);
        _data_end = .;
    } > ram AT > rom

    .bss (NOLOAD): {
        *(.bss*);
        *(COMMON);
        _bss_end = .;
    } > ram

    .bootstrap ORIGIN(bootstrap): AT(ORIGIN(bootstrap)) {
        *bootstrap.o(.text);
        . = ORIGIN(bootstrap) + LENGTH(bootstrap);
    } > bootstrap =0xff
}