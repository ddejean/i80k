// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com

#include <stdint.h>
#include <sys/types.h>

#include "list.h"

#ifndef _BLKDEV_H_
#define _BLKDEV_H_

// Block number/block address type.
typedef uint32_t block_t;

struct blkdev {
    // Handle for the list of block devices.
    struct list_node node;
    // Name of the block device.
    char *name;

    // Size of a block in bytes.
    unsigned int block_size;
    // Shift of the block size.
    unsigned int block_shift;
    // Total number of blocks on the device.
    block_t block_count;

    // Driver's private data.
    void *drv_data;

    // Block device functions.
    int (*read_block)(const struct blkdev *dev, void *buf, block_t block,
                      size_t count);
    int (*write_block)(const struct blkdev *dev, const void *buf, block_t block,
                       size_t count);
    int (*read)(const struct blkdev *dev, void *buf, off_t offset, size_t len);
    int (*write)(const struct blkdev *dev, const void *buf, off_t offset,
                 size_t len);
};

// blk_register registers |dev| as a block device.
void blk_register(struct blkdev *dev);

// blk_register_subdevice registers |dev| as a subdevice.
void blk_register_subdevice(struct blkdev *dev);

// blk_unregister removes the device |name| from the block device system and
// returns it.
struct blkdev *blk_unregister(const char *name);

// blk_open returns the block device associated with |name|.
struct blkdev *blk_open(const char *name);

// blk_read_block reads |count| blocks starting at block |offset| from |dev|.
// Returns the number of bytes read or a negative value on error.
int blk_read_block(const struct blkdev *dev, void *buf, block_t block,
                   size_t count);

// blk_write_block writes |count| blocks starting at block |offset| to |dev|.
// Returns the number of bytes written or a negative value on error.
int blk_write_block(const struct blkdev *dev, const void *buf, block_t block,
                    size_t count);

// blk_read reads |len| bytes starting at bytes |offset| from |dev|.
// Returns the number of bytes read or a negative value on error.
int blk_read(const struct blkdev *dev, void *buf, off_t offset, size_t len);

// blk_write writes |len| bytes starting at bytes |offset| from |dev|.
// Returns the number of bytes written or a negative value on error.
int blk_write(const struct blkdev *dev, const void *buf, off_t offset,
              size_t len);

size_t blk_block_trim_range(const struct blkdev *dev, block_t block,
                            size_t count);
size_t blk_trim_range(const struct blkdev *dev, off_t offset, size_t count);

#endif  // _BLKDEV_H_
