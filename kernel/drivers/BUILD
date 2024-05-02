# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_library")

# Drivers for the board revision 2.
filegroup(
    name = "srcs-rev2",
    srcs = [
        "p8251.c",
        "uart_int.S",
    ],
)

# Drivers for the board revision 3.
filegroup(
    name = "srcs-rev3",
    srcs = [
        "pc16550.c",
        "uart_int.S",
    ],
)

# Drivers for the board revision 3.
filegroup(
    name = "srcs-vboard",
    srcs = [
        "pc16550.c",
        "uart_int.S",
    ],
)

board_rev3_drivers = [
    "//kernel/drivers/block/cfi",
    "//kernel/drivers/block/cf20",
    "//kernel/drivers/fs/ext2",
]

i80k_library(
    name = "drivers",
    srcs = select({
        "//platforms/board:rev2": [":srcs-rev2"],
        "//platforms/board:rev3": [":srcs-rev3"],
        "//platforms/board:vboard": [":srcs-vboard"],
    }),
    hdrs = glob(["include/*.h"]),
    includes = ["include/"],
    visibility = ["//kernel:__pkg__"],
    deps = [
        ":libdriver",
        "//kernel/arch",
        "//kernel/boards:board",
        "//kernel/core",
    ] + select({
        "//platforms/board:rev3": board_rev3_drivers,
        "//conditions:default": [],
    }),
    alwayslink = True,
)

i80k_library(
    name = "libdriver",
    srcs = [
        "driver.c",
        "driver.h",
    ],
    hdrs = ["driver.h"],
    includes = ["."],
    visibility = [
        ":__subpackages__",
        "//kernel:__pkg__",
    ],
)
