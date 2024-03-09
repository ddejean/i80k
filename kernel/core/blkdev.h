// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com

#include <stdint.h>

#include "list.h"

#ifndef _BLKDEV_H_
#define _BLKDEV_H_

struct blkdev {
    // Handle for the list of block devices.
    struct list_node node;
    // Name of the block device.
    char *name;

    // Size of a block in bytes.
    unsigned int block_size;
    // Total number of blocks on the device.
    uint32_t block_count;

    // Driver's private data.
    void *drv_data;

    // Block device functions.
    int (*read_block)(struct blkdev *dev, void *buf, uint32_t block,
                      unsigned int count);
    int (*write_block)(struct blkdev *dev, void *buf, uint32_t block,
                       unsigned int count);
};

// blk_register registers |dev| as a block device.
void blk_register(struct blkdev *dev);

// blk_unregister removes the device |name| from the block device system and
// returns it.
struct blkdev *blk_unregister(const char *name);

// blk_open returns the block device associated with |name|.
struct blkdev *blk_open(const char *name);

int blk_read_block(struct blkdev *dev, void *buf, uint32_t block,
                   unsigned int count);

int blk_write_block(struct blkdev *dev, void *buf, uint32_t block,
                    unsigned int count);

unsigned int blk_block_trim_range(struct blkdev *dev, uint32_t block,
                                  unsigned int count);
#endif  // _BLKDEV_H_
