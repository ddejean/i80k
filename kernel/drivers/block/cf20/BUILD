# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_library")

i80k_library(
    name = "cf20",
    srcs = glob([
        "*.c",
        "*.h",
        "*.S",
    ]),
    visibility = ["//kernel/drivers:__pkg__"],
    deps = [
        "//kernel/arch",
        "//kernel/boards:board",
        "//kernel/core",
        "//kernel/drivers:libdriver",
    ],
    alwayslink = True,
)
