# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Drivers for the board revision 2.
filegroup(
    name = "srcs-rev2",
    srcs = [
        "p8251.c",
        "p8254.c",
        "p8259a.c",
        "uart_int.S",
    ],
)

# Drivers for the board revision 3.
filegroup(
    name = "srcs-rev3",
    srcs = [
        "block/cf20/cf20.c",
        "block/cf20/cf20_cmds.c",
        "block/cf20/cf20_defs.h",
        "p8254.c",
        "p8259a.c",
        "pc16550.c",
        "uart_int.S",
    ],
)

# Drivers for the board revision 3.
filegroup(
    name = "srcs-vboard",
    srcs = [
        "p8254.c",
        "p8259a.c",
        "pc16550.c",
        "uart_int.S",
    ],
)

cc_library(
    name = "drivers",
    srcs = select({
        "//platforms/board:rev2": [":srcs-rev2"],
        "//platforms/board:rev3": [":srcs-rev3"],
        "//platforms/board:vboard": [":srcs-vboard"],
    }),
    hdrs = glob(["include/*.h"]),
    includes = ["include/"],
    visibility = ["//visibility:public"],
    deps = [
        ":libdriver",
        "//kernel/arch",
        "//kernel/boards:board",
        "//kernel/core",
        "//kernel/utils",
        "//libc:c",
    ],
    alwayslink = True,
)

cc_library(
    name = "libdriver",
    srcs = [
        "driver.c",
        "driver.h",
    ],
    hdrs = ["driver.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = [
        "//libc:c",
    ],
)
