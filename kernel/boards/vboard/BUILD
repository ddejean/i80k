# Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_library")

i80k_library(
    name = "board-vboard",
    srcs = glob([
        "*.h",
        "*.c",
        "*.S",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    includes = ["."],
    visibility = ["//kernel/boards:__pkg__"],
    deps = [
        "//kernel/boards:libboard",
        "//kernel/boards/drivers:p8254",
        "//kernel/boards/drivers:p8259a",
    ],
    # Required to ensure bootstrap and crt0 files are included even if their
    # symbols are not referenced anywhere.
    alwayslink = True,
)

filegroup(
    name = "lds",
    srcs = ["kernel.lds"],
    visibility = ["//kernel/boards:__pkg__"],
)
