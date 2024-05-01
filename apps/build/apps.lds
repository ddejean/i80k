/*
 * Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>
 */

OUTPUT_FORMAT(binary)
ENTRY(main)

MEMORY {
    /* Binary memory layout */
    bin (rwx) : ORIGIN = 0x00000, LENGTH = 0x20000
    /* Code segment */
    code (rx) : ORIGIN = 0x00000, LENGTH = 0x10000
    /* Data segment */
    data (rw) : ORIGIN = 0x00000, LENGTH = 0x10000
}

SECTIONS {
    .hdr : {
        LONG(0xDEADBEEF);
        LONG(__stext);
        LONG(__ldata);
        LONG(__lbss);
        LONG(main);
    } AT > bin

    .text : {
        __stext = .;
        *(.text) *(.text.*)
        . = ALIGN(2);
        __etext = .;
    } > code AT > bin
    __ltext = __etext - __stext;

    .data : {
        __sdata = .;
        *(.rodata) *(.rodata.*)
        *(.data) *(.data.*)
        __edata = .;
    } > data AT > bin
    __ldata = __edata - __sdata;

    .bss (NOLOAD) : {
        __sbss = .;
        *(.bss) *(.bss.*)
        *(COMMON)
        . = ALIGN(2);
        __ebss = .;
    } > data AT > bin
    __lbss = __ebss - __sbss;
}
