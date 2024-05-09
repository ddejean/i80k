// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com

#include "subdev.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

struct blksubdev {
    // Original block device implementation.
    struct blkdev dev;

    // Parent block device.
    const struct blkdev *parent;
    // Offset of the subdevice on the block device.
    block_t offset;
};

int subdev_read_block(const struct blkdev *dev, void *buf, block_t block,
                      size_t count);
int subdev_write_block(const struct blkdev *dev, const void *buf, block_t block,
                       size_t count);
int subdev_read(const struct blkdev *dev, void *buf, off_t offset, size_t len);
int subdev_write(const struct blkdev *dev, const void *buf, off_t offset,
                 size_t len);

// Size of the Master Boot Record in bytes.
#define MBR_SIZE 512

struct chs {
    uint8_t c;
    uint8_t h;
    uint8_t s;
};

struct mbr_entry {
    uint8_t status;
    struct chs chs_start;
    uint8_t type;
    struct chs chs_end;
    uint32_t lba_start;
    uint32_t lba_len;
};

static int mbr_is_valid_entry(const struct blkdev *dev,
                              const struct mbr_entry *entry) {
    if (entry->type == 0) return ERR_NO_DEV;
    if (entry->status != 0x80 && entry->status != 0x00) return ERR_NO_DEV;
    if (entry->lba_len == 0) return ERR_NO_DEV;
    if (entry->lba_start >= dev->block_count) return ERR_INVAL;
    if (entry->lba_start + entry->lba_len > dev->block_count) return ERR_INVAL;
    return 0;
}

static int mbr_publish_device(const struct blkdev *dev,
                              const struct mbr_entry *entry, int index) {
    if (!dev || !entry) {
        return ERR_INVAL;
    }

    if (mbr_is_valid_entry(dev, entry) < 0) {
        return ERR_NO_DEV;
    }

    char *name = calloc(12, sizeof(char));
    if (!name) {
        return ERR_NO_MEM;
    }

    struct blksubdev *subdev = calloc(1, sizeof(*subdev));
    if (!subdev) {
        free(name);
        return ERR_NO_MEM;
    }

    snprintf(name, 12, "%s%d", dev->name, index + 1);
    subdev->dev.name = name;
    subdev->dev.block_size = dev->block_size;
    subdev->dev.block_shift = dev->block_shift;
    subdev->dev.block_count = entry->lba_len;
    subdev->dev.drv_data = dev->drv_data;
    subdev->dev.read_block = subdev_read_block;
    subdev->dev.write_block = subdev_write_block;
    subdev->dev.read = subdev_read;
    subdev->dev.write = subdev_write;
    subdev->parent = dev;
    subdev->offset = entry->lba_start;

    blk_register_subdevice((struct blkdev *)subdev);
    return 0;
}

int mbr_probe(const struct blkdev *dev) {
    int len;
    int err = 0;
    uint8_t *buf = NULL;

    buf = malloc(MBR_SIZE);
    if (!buf) {
        goto error;
    }

    // Read the first sector, expected to be the MBR.
    len = blk_read(dev, buf, 0, MBR_SIZE);
    if (len != MBR_SIZE) {
        err = len < 0 ? len : ERR_IO;
        goto error;
    }

    // MBR magic check.
    if (buf[510] != 0x55 || buf[511] != 0xaa) {
        err = ERR_IO;
        goto error;
    }

    struct mbr_entry *entries = (struct mbr_entry *)(buf + 446);
    for (int i = 0; i < 4; i++) {
        mbr_publish_device(dev, &entries[i], i);
    }

error:
    free(buf);
    return err;
}

void subdev_probe(struct blkdev *dev) {
    if (mbr_probe(dev) >= 0) {
        return;
    }
    return;
}

int subdev_read_block(const struct blkdev *dev, void *buf, block_t block,
                      size_t count) {
    struct blksubdev *sdev = (struct blksubdev *)dev;

    return blk_read_block(sdev->parent, buf, block + sdev->offset, count);
}

int subdev_write_block(const struct blkdev *dev, const void *buf, block_t block,
                       size_t count) {
    struct blksubdev *sdev = (struct blksubdev *)dev;

    return blk_write_block(sdev->parent, buf, block + sdev->offset, count);
}

int subdev_read(const struct blkdev *dev, void *buf, off_t offset, size_t len) {
    struct blksubdev *sdev = (struct blksubdev *)dev;

    return blk_read(sdev->parent, buf, offset + sdev->offset * dev->block_size,
                    len);
}

int subdev_write(const struct blkdev *dev, const void *buf, off_t offset,
                 size_t len) {
    struct blksubdev *sdev = (struct blksubdev *)dev;

    return blk_write(sdev->parent, buf, offset + sdev->offset * dev->block_size,
                     len);
}