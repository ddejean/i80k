// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"

#include "devices.h"

// PIC definition.
struct io_device pic = {
    .port = 0x20,
};

void board_initialize() { board_register_io_dev(IO_DEV_PIC_MASTER, &pic); }
