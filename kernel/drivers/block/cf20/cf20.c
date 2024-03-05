// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "cf20.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "blkdev.h"
#include "cf20_defs.h"
#include "cpu.h"
#include "devices.h"

int cf20_read_block(struct blkdev *dev, void *buf, uint32_t block,
                    unsigned int count);
int cf20_write_block(struct blkdev *dev, void *buf, uint32_t block,
                     unsigned int count);

void cf20_initialize(void) {
    struct io_device *io_dev;
    struct cf20_private *pdev = NULL;
    struct cf20_identity *cf_id = NULL;
    struct blkdev *dev = NULL;

    io_dev = board_get_io_dev(IO_DEV_CF);
    if (!io_dev) {
        // No device declared, nothing to do.
        return;
    }

    // Allocate the private driver structure.
    pdev = malloc(sizeof(*pdev));
    if (!pdev) {
        printf("CF: failed to allocate driver private struct\n");
        return;
    }

    // Fill driver private's data.
    pdev->regs.data = REG_DATA(io_dev->port);
    pdev->regs.error = REG_ERROR(io_dev->port);
    pdev->regs.features = REG_FEATURES(io_dev->port);
    pdev->regs.sector_count = REG_SECTOR_COUNT(io_dev->port);
    pdev->regs.lba_low = REG_LBA_LOW(io_dev->port);
    pdev->regs.lba_mid = REG_LBA_MID(io_dev->port);
    pdev->regs.lba_high = REG_LBA_HIGH(io_dev->port);
    pdev->regs.card_head = REG_CARD_HEAD(io_dev->port);
    pdev->regs.cmd = REG_CMD(io_dev->port);
    pdev->regs.status = REG_STATUS(io_dev->port);

    // If the device is wired on a 8 bits bus, the 8 bits mode has to be enabled
    // before any other communication.
    if (io_dev->u.cf.is_8bit) {
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
    dev->block_count = cf_id->cur_capacity;
    dev->drv_data = pdev;
    dev->read_block = cf20_read_block;
    dev->write_block = cf20_write_block;

    // Finally register the block device and leave.
    blk_register(dev);
    return;

error:
    free(dev);
    free(cf_id);
    free(pdev);
}

int cf20_read_block(struct blkdev *dev, void *buf, uint32_t block,
                    unsigned int count) {
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

    uint8_t *buffer = (uint8_t *)buf;
    for (unsigned int sector = 0; sector < count; sector++) {
        while (inb(pdev->regs.status) & SR_BSY)
            ;
        while ((inb(pdev->regs.status) & SR_DRQ) != SR_DRQ)
            ;

        for (int byte = 0; byte < CF20_SECTOR_SIZE; byte += 2) {
            buffer[sector * CF20_SECTOR_SIZE + byte + 1] = inb(pdev->regs.data);
            buffer[sector * CF20_SECTOR_SIZE + byte] = inb(pdev->regs.data);
        }
    }
    return 0;
}

int cf20_write_block(struct blkdev *dev, void *buf, uint32_t block,
                     unsigned int count) {
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

    uint8_t *buffer = (uint8_t *)buf;
    for (unsigned int sector = 0; sector < count; sector++) {
        while (inb(pdev->regs.status) & SR_BSY)
            ;
        while ((inb(pdev->regs.status) & SR_DRQ) != SR_DRQ)
            ;

        for (int byte = 0; byte < CF20_SECTOR_SIZE; byte += 2) {
            outb(pdev->regs.data, buffer[sector * CF20_SECTOR_SIZE + byte + 1]);
            outb(pdev->regs.data, buffer[sector * CF20_SECTOR_SIZE + byte]);
        }
    }
    return 0;
}