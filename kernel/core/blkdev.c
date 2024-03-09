// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "blkdev.h"

#include <string.h>

#include "list.h"

// List of registered block devices.
static struct list_node devices = LIST_INITIAL_VALUE(devices);

void blk_register(struct blkdev *dev) {
    if (!dev) {
        return;
    }
    list_add_tail(&devices, &dev->node);
}

struct blkdev *blk_unregister(const char *name) {
    if (!name) {
        return NULL;
    }
    struct blkdev *dev;
    list_for_every_entry(&devices, dev, struct blkdev, node) {
        if (!strcmp(dev->name, name)) {
            list_delete(&dev->node);
            return dev;
        }
    }
    return NULL;
}

struct blkdev *blk_open(const char *name) {
    if (!name) {
        return NULL;
    }
    struct blkdev *dev;
    list_for_every_entry(&devices, dev, struct blkdev, node) {
        if (!strcmp(dev->name, name)) {
            return dev;
        }
    }
    return NULL;
}

unsigned int blk_block_trim_range(struct blkdev *dev, uint32_t block,
                                  unsigned int count) {
    if (block >= dev->block_count) {
        return 0;
    }
    if (!count) {
        return 0;
    }
    if (block + count > dev->block_count) {
        count = dev->block_count - block;
    }
    return count;
}

int blk_read_block(struct blkdev *dev, void *buf, uint32_t block,
                   unsigned int count) {
    if (!dev || !dev->read_block || !buf) {
        return -1;
    }
    count = blk_block_trim_range(dev, block, count);
    if (!count) {
        return 0;
    }
    return dev->read_block(dev, buf, block, count);
}

int blk_write_block(struct blkdev *dev, void *buf, uint32_t block,
                    unsigned int count) {
    if (!dev || !dev->write_block || !buf) {
        return -1;
    }
    count = blk_block_trim_range(dev, block, count);
    if (!count) {
        return 0;
    }
    return dev->write_block(dev, buf, block, count);
}
