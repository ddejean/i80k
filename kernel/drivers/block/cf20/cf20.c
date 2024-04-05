// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "blkdev.h"
#include "board.h"
#include "cf20_defs.h"
#include "cpu.h"
#include "devices.h"
#include "driver.h"
#include "error.h"
#include "interrupts.h"
#include "list.h"
#include "scheduler.h"

// List of all the I/O requests waiting to be satisfied.
struct list_node requests = LIST_INITIAL_VALUE(requests);

// List of the bio request possible states.
typedef enum {
    READ_CMD,
    READ_SECTOR,
    WRITE_CMD,
    WRITE_SECTOR,
    DONE,
    ERROR,
} bio_state_t;

#define BIO_SEND(s) ((s) == READ_CMD || (s) == WRITE_CMD)
#define BIO_RW_SECTOR(s) ((s) == READ_SECTOR || (s) == WRITE_SECTOR)
#define BIO_DONE(s) ((s) == DONE)

struct bio_request {
    const struct cf20_private *pdev;
    bio_state_t state;
    block_t block;
    size_t count;
    uint8_t *buf;
};

extern void cf20_int_handler(void);

void cf20_handler(void) {
    struct task *task;
    struct bio_request *io_req;

    irq_ack(5);

    // No requests to handle, this is a spurious interruption.
    if (list_is_empty(&requests)) {
        return;
    }

    task = list_peek_head_type(&requests, struct task, node);
    io_req = (struct bio_request *)task->wait_state;

    // The command was already sent to the device, just read the sector that's
    // ready.
    if (BIO_RW_SECTOR(io_req->state)) {
        if (io_req->state == READ_SECTOR) {
            cf20_read_sector(io_req->pdev, io_req->buf);
        } else if (io_req->state == WRITE_SECTOR) {
            cf20_write_sector(io_req->pdev, io_req->buf);
        }
        io_req->block++;
        io_req->count--;
        io_req->buf += io_req->pdev->sector_sz;
        if (io_req->count == 0) {
            io_req->state = DONE;
        }
    }

    // I/O request is finished, unblock the task waiting for it and prepare the
    // next one.
    if (BIO_DONE(io_req->state)) {
        printf("cf20: wake up pid=%d\n", task->pid);
        list_delete(&task->node);
        scheduler_wake_up(task);

        // Prepare the next task.
        if (list_is_empty(&requests)) {
            // No next task, nothing else to do.
            return;
        }
        task = list_peek_head_type(&requests, struct task, node);
        io_req = (struct bio_request *)task->wait_state;
    }

    if (BIO_SEND(io_req->state)) {
        if (io_req->state == READ_CMD) {
            cf20_send_read_sectors(io_req->pdev, io_req->block, io_req->count);
        } else if (io_req->state == WRITE_CMD) {
            cf20_send_write_sectors(io_req->pdev, io_req->block, io_req->count);
        }
    }
}

int cf20_read_block(const struct blkdev *dev, void *buf, block_t block,
                    size_t count) {
    struct cf20_private *pdev = (struct cf20_private *)dev->drv_data;

    if (!count || count > 256 || !buf) {
        return ERR_INVAL;
    }

    struct bio_request *io_req = calloc(1, sizeof(*io_req));
    io_req->pdev = pdev;
    io_req->block = block;
    io_req->count = count;
    io_req->buf = buf;

    if (list_is_empty(&requests)) {
        // Send a read sector command to the card.
        cf20_send_read_sectors(pdev, block, count);
        io_req->state = READ_SECTOR;
    } else {
        io_req->state = READ_CMD;
    }

    // Block waiting for the answer.
    scheduler_sleep_on(&requests, io_req);

    int bytes_read = (count - io_req->count) * pdev->sector_sz;
    free(io_req);
    return bytes_read;
}

int cf20_write_block(const struct blkdev *dev, const void *buf, block_t block,
                     size_t count) {
    struct cf20_private *pdev = (struct cf20_private *)dev->drv_data;

    if (!count || count > 256 || !buf) {
        return ERR_INVAL;
    }

    struct bio_request *io_req = calloc(1, sizeof(*io_req));
    io_req->pdev = pdev;
    io_req->block = block;
    io_req->count = count;
    io_req->buf = (void *)buf;

    if (list_is_empty(&requests)) {
        // Send a read sector command to the card.
        cf20_send_write_sectors(pdev, block, count);
        io_req->state = WRITE_SECTOR;
    } else {
        io_req->state = WRITE_CMD;
    }

    // Block waiting for the answer.
    scheduler_sleep_on(&requests, io_req);

    int bytes_written = (count - io_req->count) * pdev->sector_sz;
    free(io_req);
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
    pdev = calloc(1, sizeof(*pdev));
    if (!pdev) {
        printf("CF: failed to allocate driver private struct\n");
        return false;
    }

    // Fill driver private's data.
    pdev->sector_sz = CF20_SECTOR_SIZE;
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
    pdev->regs.dev_ctrl = REG_STATUS(cfg->port);

    // If the device is wired on a 8 bits bus, the 8 bits mode has to be enabled
    // before any other communication.
    if (cfg->is_8bit) {
        if (!cf20_set_feature(pdev, FEAT_8BIT_ENABLE, 0)) {
            printf("CF: failed to enable 8 bits mode.\n");
            goto error;
        }
    }

    cf_id = calloc(1, sizeof(*cf_id));
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

    // Enable interrupts.
    interrupts_handle(interrupts_from_irq(cfg->irq), KERNEL_CS,
                      cf20_int_handler);
    irq_enable(cfg->irq);
    outb(pdev->regs.dev_ctrl, 0);

    // Create the block device.
    dev = calloc(1, sizeof(*dev));
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
    free(cf_id);
    return true;

error:
    free(dev);
    free(cf_id);
    free(pdev);
    return false;
}

DRIVER(cf20, cf20_probe);
