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