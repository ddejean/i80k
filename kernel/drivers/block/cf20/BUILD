# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "cf20",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    visibility = ["//kernel/drivers:__pkg__"],
    deps = [
        "//kernel/arch",
        "//kernel/boards:board",
        "//kernel/core",
        "//kernel/drivers:libdriver",
        "//libc:c",
    ],
    alwayslink = True,
)
