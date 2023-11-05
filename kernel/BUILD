# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

filegroup(
    name = "srcs",
    srcs = glob(
        [
            "*.S",
            "*.h",
            "*.c",
        ],
        [
            "p8251.*",
            "pc16550.*",
        ],
    ),
)

filegroup(
    name = "drivers",
    srcs = select({
        "//kernel/boards:8088-rev2": [
            "p8251.c",
            "p8251.h",
        ],
        "//kernel/boards:8088-rev3": [
            "pc16550.c",
            "pc16550.h",
        ],
    }),
)

cc_binary(
    name = "kernel.bin",
    srcs = [
        ":drivers",
        ":srcs",
    ],
    additional_linker_inputs = ["//kernel/boards:lds"],
    linkopts = [
        "-nostdlib",
        "-T $(location //kernel/boards:lds)",
    ],
    deps = [
        "//kernel/boards:board",
        "//kernel/utils",
        "//libc:c",
    ],
)
