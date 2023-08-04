# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

filegroup(
    name = "srcs",
    srcs = glob([
        "*.S",
        "*.h",
        "*.c",
    ]),
)

cc_binary(
    name = "kernel.bin",
    srcs = [
        ":srcs",
    ],
    additional_linker_inputs = ["//boards:lds"],
    linkopts = [
        "-nostdlib",
        "-T $(location //boards:lds)",
    ],
    deps = [
        "//boards:board",
        "//lib:c",
        "//utils",
    ],
)
