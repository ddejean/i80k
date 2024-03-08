/* Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com> */

OUTPUT_FORMAT(binary)

MEMORY {
  /* IDT is located at address 0x0 in the first segment. */
  idt       (rwx) : ORIGIN = 0x0000, LENGTH = 0x0400
  /* The usable RAM in the first segment is right after the IDT. */
  ram       (rwx) : ORIGIN = 0x0400, LENGTH = 0x7FF0
  /* The ROM is physically located at address F000:8000. */
  rom       (rx)  : ORIGIN = 0xE0000, LENGTH = 0x1FFF0
  /* The bootstrap code is physically and logically located at address F000:FFF0. */
  bootstrap (rx)  : ORIGIN = 0xFFFF0, LENGTH = 0x0010
}

SECTIONS {
    /*
     * Interrupt Descriptor Table must be located at address 0000:0000 but won't
     * be loaded from the ROM, that's why it uses NOLOAD.
     */
    .idt (NOLOAD) : {
        *(.idt*);
    } > idt

    /*
     * .text and .data sections are placed at the begining of the ROM will be
     * relocated to address 0000:0400 in RAM, just after the IDT.
     */
    .text : {
        _text_start = .;
        *crt0.o(.text*);
        EXCLUDE_FILE(*crt0.o *bootstrap.o) *(.text*);
        /*
         * Align the end of the section to ensure the following section will be
         * physically located  right after this one. Note: it will be filled with
         * 0xff.
         */
        . = ALIGN(4);
        _text_end = .;
    } > ram AT > rom =0xff

    .data : ALIGN(4) {
        _data_start = .;
        *(.rodata*);
        _devices_start = .;
        *(.devices);
        _devices_end = .;
        _drivers_start = .;
        *(.drivers);
        _drivers_end = .;
        *(.data*);

        /* See comment in previous section. */
        . = ALIGN(4);
        _data_end = .;
    } > ram AT > rom =0xff

    /*
     * .bss will also be located in the first segment right after the .data
     * section but does not need to be loaded as it contains un-initialized
     * data.
     */
    .bss (NOLOAD) : ALIGN(4) {
        _bss_start = .;
        *(.bss*);
        *(COMMON);
        _bss_end = .;
    } > ram

    /*
     * Dummy section used to fill the void in the ROM between the end of .bss
     * and the bootstrap code.
     */
    .fill : {
        BYTE(0xFF)
        . = ORIGIN(rom) + LENGTH(rom);
      } > rom =0xff

    /*
     * The bootstrap code (first code run by the CPU) needs to be located at
     * physical address F000:FFF0, thus must be placed at offset 0x7FF0 in the
     * ROM binary.
     */
    .bootstrap : {
        *bootstrap.o(.text);
        . = ORIGIN(bootstrap) + LENGTH(bootstrap);
    } > bootstrap AT > bootstrap =0xff
}