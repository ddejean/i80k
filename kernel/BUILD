# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_binary")

i80k_binary(
    name = "kernel.bin",
    srcs = glob(
        [
            "*.S",
            "*.h",
            "*.c",
        ],
    ),
    deps = [
        "//kernel/arch",
        "//kernel/boards:board",
        "//kernel/core",
        "//kernel/drivers",
        "//third_party/async_xmodem",
    ],
)
