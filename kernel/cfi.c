// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "cfi.h"

#include <stdint.h>
#include <stdio.h>

#include "clock.h"

// Manufacturer ID list.
#define CFI_VENDOR_SST 0xBF

// Device ID list.
#define CFI_DEVICE_SST39SF010A 0xB5
#define CFI_DEVICE_SST39SF020A 0xB6
#define CFI_DEVICE_SST39SF040 0xB7

#define CFI_ADDR0 0x5555
#define CFI_ADDR1 0x2AAA

#define CFI_BYTE0 0xAA
#define CFI_BYTE1 0x55

#define CFI_CMD_BYTE_PROGRAM 0xA0
#define CFI_CMD_ERASE 0x80
#define CFI_CMD_ERASE_SECTOR 0x30
#define CFI_CMD_ERASE_CHIP 0x10
#define CFI_CMD_SOFT_ID_ENTRY 0x90
#define CFI_CMD_SOFT_ID_EXIT 0xF0

#define CFI_TOGGLE_BIT (1 << 6)
#define CFI_POLLING_BIT (1 << 7)

typedef volatile uint8_t __far *cfi_ptr_t;

cfi_ptr_t cfi_reg0 = (cfi_ptr_t)(0xE0000000 + CFI_ADDR0);
cfi_ptr_t cfi_reg1 = (cfi_ptr_t)(0xE0000000 + CFI_ADDR1);
cfi_ptr_t flash_addr0 = (cfi_ptr_t)0xE0000000;
cfi_ptr_t flash_addr1 = (cfi_ptr_t)0xE0000001;

static void cfi_write_cmd(uint8_t cmd) {
    *cfi_reg0 = CFI_BYTE0;
    *cfi_reg1 = CFI_BYTE1;
    *cfi_reg0 = cmd;
}

static char *cfi_sst_device(uint8_t id) {
    switch (id) {
        case CFI_DEVICE_SST39SF010A:
            return "SST39SF010A";
        case CFI_DEVICE_SST39SF020A:
            return "SST39SF020A";
        case CFI_DEVICE_SST39SF040:
            return "SST39SF040";
        default:
            return "unknown";
    }
}

static char *cfi_device(uint8_t vendor_id, uint8_t dev_id) {
    switch (vendor_id) {
        case CFI_VENDOR_SST:
            return cfi_sst_device(dev_id);
        default:
            return "unknown";
    }
}

static void cfi_toggle_wait(cfi_ptr_t addr) {
    uint8_t byte0 = *addr;
    uint8_t byte1 = *addr;
    while ((byte0 & CFI_TOGGLE_BIT) != (byte1 & CFI_TOGGLE_BIT)) {
        byte0 = *addr;
        byte1 = *addr;
        udelay(20);
    }
}

void cfi_initialize(void) {
    uint8_t vendor_id, dev_id;
    cfi_write_cmd(CFI_CMD_SOFT_ID_ENTRY);
    vendor_id = *flash_addr0;
    dev_id = *flash_addr1;
    cfi_write_cmd(CFI_CMD_SOFT_ID_EXIT);

    printf("CFI flash: %s\n", cfi_device(vendor_id, dev_id));
}

void cfi_chip_erase(void) {
    cfi_write_cmd(CFI_CMD_ERASE);
    cfi_write_cmd(CFI_CMD_ERASE_CHIP);
    cfi_toggle_wait(flash_addr0);
}

void cfi_write(uint32_t offset, uint8_t byte) {
    (void)offset;
    (void)byte;
    cfi_write_cmd(CFI_CMD_BYTE_PROGRAM);

    cfi_ptr_t addr = flash_addr0 + offset;
    *addr = byte;
    cfi_toggle_wait(addr);
}
