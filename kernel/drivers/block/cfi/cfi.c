// Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "blkdev.h"
#include "board.h"
#include "delay.h"
#include "devices.h"
#include "driver.h"
#include "error.h"
#include "fmem.h"

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

// Private's driver data.
struct cfi_private {
    volatile u8_fptr_t base;
    volatile u8_fptr_t reg0;
    volatile u8_fptr_t reg1;
    volatile u8_fptr_t addr0;
    volatile u8_fptr_t addr1;
};

// cfi_get_block_addr return the address of the block as a far pointer.
static u8_fptr_t cfi_get_block_addr(const struct blkdev *dev, block_t block) {
    struct cfi_private *pdev = dev->drv_data;

    uint32_t offset = block * (uint32_t)dev->block_size;
    uint32_t off_high = ((offset & 0xffff0000) << 12);
    uint32_t off_low = offset & 0xffff;
    uint32_t addr = (uint32_t)pdev->base;

    return (u8_fptr_t)(addr + off_high + off_low);
}

static void cfi_toggle_wait(volatile u8_fptr_t addr) {
    uint8_t byte0;
    uint8_t byte1;
    do {
        byte0 = *addr;
        // Add a delay between the two reads to ensure the flash has enough time
        // to flip its bit.
        udelay(1);
        byte1 = *addr;
    } while ((byte0 & CFI_TOGGLE_BIT) != (byte1 & CFI_TOGGLE_BIT));
}

static void cfi_erase_block(struct cfi_private *pdev, volatile u8_fptr_t addr) {
    // Write the erase sector command.
    *(pdev->reg0) = CFI_BYTE0;
    *(pdev->reg1) = CFI_BYTE1;
    *(pdev->reg0) = CFI_CMD_ERASE;
    *(pdev->reg0) = CFI_BYTE0;
    *(pdev->reg1) = CFI_BYTE1;
    *addr = CFI_CMD_ERASE_SECTOR;

    // Wait for the erase operation to be done.
    cfi_toggle_wait(addr);
}

static void cfi_write_byte(struct cfi_private *pdev, volatile u8_fptr_t addr,
                           uint8_t byte) {
    // Program the byte.
    *(pdev->reg0) = CFI_BYTE0;
    *(pdev->reg1) = CFI_BYTE1;
    *(pdev->reg0) = CFI_CMD_BYTE_PROGRAM;
    *addr = byte;

    // Wait for the write operation to complete.
    cfi_toggle_wait(addr);
}

int cfi_read_block(const struct blkdev *dev, void *buf, block_t block,
                   size_t count) {
    struct cfi_private *pdev;

    pdev = dev->drv_data;
    if (!pdev) {
        return ERR_INVAL;
    }

    int bytes_read = 0;
    uint8_t *buffer = buf;
    for (unsigned int i = 0; i < count; i++) {
        volatile u8_fptr_t addr = cfi_get_block_addr(dev, block + i);
        fmemcpy(fmem_void_fptr(KERNEL_DS, buffer), addr, dev->block_size);
        buffer += dev->block_size;
        bytes_read += dev->block_size;
    }

    return bytes_read;
}

int cfi_write_block(const struct blkdev *dev, const void *buf, block_t block,
                    size_t count) {
    struct cfi_private *pdev;

    pdev = dev->drv_data;
    if (!pdev) {
        return ERR_INVAL;
    }

    int bytes_written = 0;
    const uint8_t *buffer = buf;
    for (unsigned int i = 0; i < count; i++) {
        volatile u8_fptr_t addr = cfi_get_block_addr(dev, block + i);
        cfi_erase_block(pdev, addr);
        for (unsigned int j = 0; j < dev->block_size; j++) {
            cfi_write_byte(pdev, addr, *buffer);
            buffer++;
            addr++;
            bytes_written++;
        }
    }

    return bytes_written;
}

static void cfi_identity(struct cfi_private *pdev, uint8_t *vendor_id,
                         uint8_t *dev_id) {
    *(pdev->reg0) = CFI_BYTE0;
    *(pdev->reg1) = CFI_BYTE1;
    *(pdev->reg0) = CFI_CMD_SOFT_ID_ENTRY;
    *vendor_id = *(pdev->addr0);
    *dev_id = *(pdev->addr1);
    *(pdev->reg0) = CFI_BYTE0;
    *(pdev->reg1) = CFI_BYTE1;
    *(pdev->reg0) = CFI_CMD_SOFT_ID_EXIT;
}

bool cfi_probe(void) {
    const struct device *dev;
    const struct cfi_flash *cfg;
    struct cfi_private *pdev;
    struct blkdev *bdev;
    uint8_t vendor_id, chip_id;

    dev = board_get_by_driver("cfi");
    if (!dev) {
        // No device found, nothing to do.
        return false;
    }

    cfg = dev->config;
    if (!cfg) {
        printf("CFI: failed to obtain device configuration.\n");
        return false;
    }

    pdev = calloc(1, sizeof(*pdev));
    if (!pdev) {
        printf("CFI: failed to allocate private data.\n");
        return false;
    }

    pdev->base = (volatile u8_fptr_t)cfg->base_addr;
    pdev->reg0 = (volatile u8_fptr_t)(cfg->base_addr + CFI_ADDR0);
    pdev->reg1 = (volatile u8_fptr_t)(cfg->base_addr + CFI_ADDR1);
    pdev->addr0 = (volatile u8_fptr_t)cfg->base_addr;
    pdev->addr1 = (volatile u8_fptr_t)(cfg->base_addr + 1);

    // Check chip identity.
    cfi_identity(pdev, &vendor_id, &chip_id);
    if (cfg->vendor_id != vendor_id || cfg->chip_id != chip_id) {
        printf("CFI: vendor or chip id does not match.\n");
        goto error;
    }

    bdev = calloc(1, sizeof(*bdev));
    if (!bdev) {
        printf("CFI: failed to allocate block device.\n");
        goto error;
    }

    bdev->name = "mtd0";
    bdev->block_size = cfg->sector_size;
    bdev->block_shift = cfg->sector_shift;
    bdev->block_count = cfg->sector_count;
    bdev->drv_data = pdev;
    bdev->read_block = cfi_read_block;
    bdev->write_block = cfi_write_block;

    printf("CFI: %s flash, %lu sectors of %u bytes, capacity: %luKB\n",
           cfi_device(vendor_id, chip_id), bdev->block_count, bdev->block_size,
           (bdev->block_count * bdev->block_size) / 1024);

    blk_register(bdev);

    return true;

error:
    free(pdev);
    return false;
}

// Register the driver for the probing system.
DRIVER(cfi, cfi_probe);
