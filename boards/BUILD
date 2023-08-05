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

cc_library(
    name = "board",
    srcs = select({
        ":8088-rev2": ["//boards/8088-rev2:srcs"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    hdrs = select({
        ":8088-rev2": ["//boards/8088-rev2:hdrs"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    includes = select({
        ":8088-rev2": ["8088-rev2"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    # Required to ensure bootstrap and crt0 files are included even if their
    # symbols are not referenced anywhere.
    alwayslink = True,
)

filegroup(
    name = "lds",
    srcs = select({
        ":8088-rev2": ["//boards/8088-rev2:lds"],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
)
