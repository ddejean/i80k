// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "init.h"

#include <stdio.h>
#include <sys/mount.h>

void init(void) {
    int err;
    printf("Starting init...\n");

    err = mount("sda1", "/rootfs", "ext2", 0, 0);
    if (err < 0) {
        printf("init: mount failed (error %d)\n", err);
        return;
    }
}
