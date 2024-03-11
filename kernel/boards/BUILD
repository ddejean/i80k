# Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

# Build configuration for the 8088 board revision 2.
config_setting(
    name = "8088-rev2",
    constraint_values = [
        "//platforms/cpu:8088",
        "//platforms/board:rev2",
    ],
)

# Build configuration for the 8088 board revision 3.
config_setting(
    name = "8088-rev3",
    constraint_values = [
        "//platforms/cpu:8088",
        "//platforms/board:rev3",
    ],
)

# Build configuration for the 8088 virtual board.
config_setting(
    name = "8088-vboard",
    constraint_values = [
        "//platforms/cpu:8088",
        "//platforms/board:vboard",
    ],
)

cc_library(
    name = "board",
    visibility = ["//kernel:__subpackages__"],
    deps = select({
        ":8088-rev2": ["//kernel/boards/8088-rev2:board-rev2"],
        ":8088-rev3": ["//kernel/boards/8088-rev3:board-rev3"],
        ":8088-vboard": ["//kernel/boards/vboard:board-vboard"],
        "//conditions:default": ["@platforms//:incompatible"],
    }) + [
        ":libboard",
    ],
    # Required to ensure bootstrap and crt0 files are included even if their
    # symbols are not referenced anywhere.
    alwayslink = True,
)

cc_library(
    name = "libboard",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    includes = ["."],
    visibility = [":__subpackages__"],
    deps = [
        "//kernel/arch",
        "//libc:c",
    ],
)

filegroup(
    name = "lds",
    srcs = select({
        ":8088-rev2": ["//kernel/boards/8088-rev2:lds"],
        ":8088-rev3": ["//kernel/boards/8088-rev3:lds"],
        ":8088-vboard": ["//kernel/boards/vboard:lds"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//kernel:__pkg__"],
)
