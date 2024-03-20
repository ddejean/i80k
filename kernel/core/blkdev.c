// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "blkdev.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "list.h"

// List of registered block devices.
static struct list_node devices = LIST_INITIAL_VALUE(devices);

int blk_default_read(const struct blkdev *dev, void *_buf, off_t offset,
                     size_t len);
int blk_default_write(const struct blkdev *dev, const void *_buf, off_t offset,
                      size_t len);

void blk_register(struct blkdev *dev) {
    if (!dev) {
        return;
    }
    if (!dev->read) {
        dev->read = blk_default_read;
    }
    if (!dev->write) {
        dev->write = blk_default_write;
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

size_t blk_block_trim_range(const struct blkdev *dev, block_t block,
                            size_t count) {
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

int blk_read_block(const struct blkdev *dev, void *buf, block_t block,
                   size_t count) {
    if (!dev || !dev->read_block || !buf) {
        return ERR_INVAL;
    }
    count = blk_block_trim_range(dev, block, count);
    if (!count) {
        return 0;
    }
    return dev->read_block(dev, buf, block, count);
}

int blk_write_block(const struct blkdev *dev, const void *buf, block_t block,
                    size_t count) {
    if (!dev || !dev->write_block || !buf) {
        return ERR_INVAL;
    }
    count = blk_block_trim_range(dev, block, count);
    if (!count) {
        return 0;
    }
    return dev->write_block(dev, buf, block, count);
}

size_t blk_trim_range(const struct blkdev *dev, off_t offset, size_t len) {
    const off_t total_size = dev->block_size * dev->block_count;
    if (offset >= total_size) {
        return 0;
    }
    if (!len) {
        return 0;
    }
    if (offset + len > total_size) {
        len = total_size - offset;
    }
    return len;
}

int blk_read(const struct blkdev *dev, void *buf, off_t offset, size_t len) {
    if (!dev || !dev->read || !buf) {
        return ERR_INVAL;
    }
    len = blk_trim_range(dev, offset, len);
    if (!len) {
        return 0;
    }
    return dev->read(dev, buf, offset, len);
}

int blk_write(const struct blkdev *dev, const void *buf, off_t offset,
              size_t len) {
    if (!dev || !dev->read || !buf) {
        return ERR_INVAL;
    }
    len = blk_trim_range(dev, offset, len);
    if (!len) {
        return 0;
    }
    return dev->write(dev, buf, offset, len);
}

int blk_default_read(const struct blkdev *dev, void *_buf, off_t offset,
                     size_t len) {
    uint8_t *buf = (uint8_t *)_buf;
    int bytes_read = 0;
    block_t block;
    int err = 0;
    uint8_t *temp = NULL;

    // Temporary buffer for partial block transfers.
    temp = malloc(dev->block_size);
    if (!temp) {
        return ERR_NO_MEM;
    }

    // Find the starting block.
    block = offset / dev->block_size;

    // Handle partial first block.
    if ((offset % dev->block_size) != 0) {
        // Read in the block.
        err = blk_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        /* copy what we need */
        size_t block_offset = offset % dev->block_size;
        size_t tocopy = MIN(dev->block_size - block_offset, len);
        memcpy(buf, temp + block_offset, tocopy);

        /* increment our buffers */
        buf += tocopy;
        len -= tocopy;
        bytes_read += tocopy;
        block++;
    }

    // Handle middle blocks.
    uint32_t num_blocks = len >> dev->block_shift;
    err = blk_read_block(dev, buf, block, num_blocks);
    if (err < 0) {
        goto err;
    } else if ((size_t)err != dev->block_size * num_blocks) {
        err = ERR_IO;
        goto err;
    }
    buf += err;
    len -= err;
    bytes_read += err;
    block += num_blocks;

    // Handle partial last block.
    if (len > 0) {
        // Read the block.
        err = blk_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        // copy the partial block from our temp buffer.
        memcpy(buf, temp, len);

        bytes_read += len;
    }

err:
    free(temp);
    // Return error or bytes read.
    return (err >= 0) ? bytes_read : err;
}

int blk_default_write(const struct blkdev *dev, const void *_buf, off_t offset,
                      size_t len) {
    const uint8_t *buf = (const uint8_t *)_buf;
    int bytes_written = 0;
    block_t block;
    int err = 0;
    uint8_t *temp = NULL;

    // Temporary buffer for partial block transfers.
    temp = malloc(dev->block_size);
    if (!temp) {
        return ERR_NO_MEM;
    }

    // Find the starting block.
    block = offset / dev->block_size;

    // Handle partial first block.
    if ((offset % dev->block_size) != 0) {
        // Read in the block.
        err = blk_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        // Copy what we need.
        size_t block_offset = offset % dev->block_size;
        size_t tocopy = MIN(dev->block_size - block_offset, len);
        memcpy(temp + block_offset, buf, tocopy);

        // Write it back out.
        err = blk_write_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        // Increment our buffers.
        buf += tocopy;
        len -= tocopy;
        bytes_written += tocopy;
        block++;
    }

    block_t block_count = len >> dev->block_shift;
    err = blk_write_block(dev, buf, block, block_count);
    if (err < 0) {
        goto err;
    } else if ((size_t)err != dev->block_size * block_count) {
        err = ERR_IO;
        goto err;
    }

    // DEBUG_ASSERT((size_t)err == (block_count * dev->block_size));

    buf += err;
    len -= err;
    bytes_written += err;
    block += block_count;

    // Handle partial last block.
    if (len > 0) {
        // Read the block.
        err = blk_read_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        // Copy the partial block from our temp buffer.
        memcpy(temp, buf, len);

        // Write it back out.
        err = blk_write_block(dev, temp, block, 1);
        if (err < 0) {
            goto err;
        } else if ((size_t)err != dev->block_size) {
            err = ERR_IO;
            goto err;
        }

        bytes_written += len;
    }

err:
    free(temp);
    // Return error or bytes written.
    return (err >= 0) ? bytes_written : err;
}
