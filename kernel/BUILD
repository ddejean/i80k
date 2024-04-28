# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

cc_binary(
    name = "kernel.bin",
    srcs = glob(
        [
            "*.S",
            "*.h",
            "*.c",
        ],
    ),
    additional_linker_inputs = ["//kernel/boards:lds"],
    features = [
        "no_stdlib",
    ],
    linkopts = [
        "-T $(location //kernel/boards:lds)",
    ],
    deps = [
        "//kernel/arch",
        "//kernel/boards:board",
        "//kernel/core",
        "//kernel/drivers",
        "//libc:c",
        "//third_party/async_xmodem",
    ],
)
