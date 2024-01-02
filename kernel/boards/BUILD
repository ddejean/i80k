# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package(default_visibility = ["//visibility:public"])

# Build configuration for the 8088 board revision 2.
config_setting(
    name = "8088-rev2",
    constraint_values = [
        "//platforms/cpu:8088",
        "//platforms/board:rev2",
    ],
)

config_setting(
    name = "8088-rev3",
    constraint_values = [
        "//platforms/cpu:8088",
        "//platforms/board:rev3",
    ],
)

config_setting(
    name = "8088-vboard",
    constraint_values = [
        "//platforms/cpu:8088",
        "//platforms/board:vboard",
    ],
)

cc_library(
    name = "board",
    srcs = glob([
        "*.h",
        "*.c",
    ]) + select({
        ":8088-rev2": ["//kernel/boards/8088-rev2:srcs"],
        ":8088-rev3": ["//kernel/boards/8088-rev3:srcs"],
        ":8088-vboard": ["//kernel/boards/vboard:srcs"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    hdrs = glob([
        "*.h",
    ]) + select({
        ":8088-rev2": ["//kernel/boards/8088-rev2:hdrs"],
        ":8088-rev3": ["//kernel/boards/8088-rev3:hdrs"],
        ":8088-vboard": ["//kernel/boards/vboard:hdrs"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    includes = ["."] + select({
        ":8088-rev2": ["8088-rev2"],
        ":8088-rev3": ["8088-rev3"],
        ":8088-vboard": ["vboard"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    deps = [
        "//libc:c",
    ],
    # Required to ensure bootstrap and crt0 files are included even if their
    # symbols are not referenced anywhere.
    alwayslink = True,
)

filegroup(
    name = "lds",
    srcs = select({
        ":8088-rev2": ["//kernel/boards/8088-rev2:lds"],
        ":8088-rev3": ["//kernel/boards/8088-rev3:lds"],
        ":8088-vboard": ["//kernel/boards/vboard:lds"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
)
