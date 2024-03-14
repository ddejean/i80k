// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "blkdev.h"
#include "cf20_defs.h"
#include "cpu.h"
#include "devices.h"
#include "driver.h"

int cf20_read_block(const struct blkdev *dev, void *buf, block_t block,
                    size_t count) {
    struct cf20_private *pdev = (struct cf20_private *)dev->drv_data;

    if (!count || count > 256 || !buf) {
        return -1;
    }

    // 0 is a special value for 256.
    outb(pdev->regs.sector_count, count < 256 ? count : 0);
    outb(pdev->regs.lba_low, block);
    outb(pdev->regs.lba_mid, block >> 8);
    outb(pdev->regs.lba_high, block >> 16);
    outb(pdev->regs.card_head,
         CHR_CARD0 | CHR_LBA | ((block >> 24) & CHR_HEAD_MASK));
    outb(pdev->regs.cmd, CMD_READ_SECTORS);

    int bytes_read = 0;
    uint8_t *buffer = (uint8_t *)buf;
    for (unsigned int sector = 0; sector < count; sector++) {
        while (inb(pdev->regs.status) & SR_BSY)
            ;
        while ((inb(pdev->regs.status) & SR_DRQ) != SR_DRQ)
            ;

        for (int byte = 0; byte < CF20_SECTOR_SIZE; byte += 2) {
            buffer[sector * CF20_SECTOR_SIZE + byte + 1] = inb(pdev->regs.data);
            buffer[sector * CF20_SECTOR_SIZE + byte] = inb(pdev->regs.data);
            bytes_read += 2;
        }
    }
    return bytes_read;
}

int cf20_write_block(const struct blkdev *dev, const void *buf, block_t block,
                     size_t count) {
    struct cf20_private *pdev = (struct cf20_private *)dev->drv_data;

    if (!count || count > 256 || !buf) {
        return -1;
    }

    // 0 is a special value for 256.
    outb(pdev->regs.sector_count, count < 256 ? count : 0);
    outb(pdev->regs.lba_low, block);
    outb(pdev->regs.lba_mid, block >> 8);
    outb(pdev->regs.lba_high, block >> 16);
    outb(pdev->regs.card_head,
         CHR_CARD0 | CHR_LBA | ((block >> 24) & CHR_HEAD_MASK));
    outb(pdev->regs.cmd, CMD_WRITE_SECTORS);

    int bytes_written = 0;
    uint8_t *buffer = (uint8_t *)buf;
    for (unsigned int sector = 0; sector < count; sector++) {
        while (inb(pdev->regs.status) & SR_BSY)
            ;
        while ((inb(pdev->regs.status) & SR_DRQ) != SR_DRQ)
            ;

        for (int byte = 0; byte < CF20_SECTOR_SIZE; byte += 2) {
            outb(pdev->regs.data, buffer[sector * CF20_SECTOR_SIZE + byte + 1]);
            outb(pdev->regs.data, buffer[sector * CF20_SECTOR_SIZE + byte]);
            bytes_written += 2;
        }
    }
    return bytes_written;
}

bool cf20_probe(void) {
    const struct device *bdev;
    const struct cf20 *cfg;
    struct cf20_private *pdev = NULL;
    struct cf20_identity *cf_id = NULL;
    struct blkdev *dev = NULL;

    bdev = board_get_by_driver("cf20");
    if (!bdev) {
        // No device declared, nothing to do.
        return false;
    }

    cfg = (struct cf20 *)bdev->config;
    if (!cfg) {
        printf("CF: no device configuration\n");
        return false;
    }

    // Allocate the private driver structure.
    pdev = malloc(sizeof(*pdev));
    if (!pdev) {
        printf("CF: failed to allocate driver private struct\n");
        return false;
    }

    // Fill driver private's data.
    pdev->regs.data = REG_DATA(cfg->port);
    pdev->regs.error = REG_ERROR(cfg->port);
    pdev->regs.features = REG_FEATURES(cfg->port);
    pdev->regs.sector_count = REG_SECTOR_COUNT(cfg->port);
    pdev->regs.lba_low = REG_LBA_LOW(cfg->port);
    pdev->regs.lba_mid = REG_LBA_MID(cfg->port);
    pdev->regs.lba_high = REG_LBA_HIGH(cfg->port);
    pdev->regs.card_head = REG_CARD_HEAD(cfg->port);
    pdev->regs.cmd = REG_CMD(cfg->port);
    pdev->regs.status = REG_STATUS(cfg->port);

    // If the device is wired on a 8 bits bus, the 8 bits mode has to be enabled
    // before any other communication.
    if (cfg->is_8bit) {
        if (!cf20_set_feature(pdev, FEAT_8BIT_ENABLE, 0)) {
            printf("CF: failed to enable 8 bits mode.\n");
            goto error;
        }
    }

    cf_id = malloc(sizeof(*cf_id));
    if (!cf_id) {
        printf("CF: failed to allocate identity struct.\n");
        goto error;
    }
    if (!cf20_identify(pdev, cf_id)) {
        printf("CF: failed to read identity.\n");
        goto error;
    }

    printf("CF: model: %s, revision: %s, serial number: %s\n", cf_id->model,
           cf_id->firmware, cf_id->serial);
    printf("CF: %lu sectors of %d bytes, capacity: %luMB\n",
           cf_id->cur_capacity, CF20_SECTOR_SIZE,
           (cf_id->cur_capacity * CF20_SECTOR_SIZE) / 1000000);

    // Check for LBA support since this driver only supports LBA mode.
    if (!(cf_id->capabilities & CAP_LBA)) {
        printf("CF: card does no support LBA\n");
        goto error;
    }

    // Create the block device.
    dev = malloc(sizeof(*dev));
    if (!dev) {
        printf("CF: failed to allocate block device.\n");
        goto error;
    }

    dev->name = "sda";
    dev->block_size = CF20_SECTOR_SIZE;
    dev->block_shift = CF20_SECTOR_SHIFT;
    dev->block_count = cf_id->cur_capacity;
    dev->drv_data = pdev;
    dev->read_block = cf20_read_block;
    dev->write_block = cf20_write_block;

    // Finally register the block device and leave.
    blk_register(dev);
    return true;

error:
    free(dev);
    free(cf_id);
    free(pdev);
    return false;
}

DRIVER(cf20, cf20_probe);
