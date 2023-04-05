cc_binary(
    name = "kernel.bin",
    srcs = glob([
        "*.S",
        "*.h",
        "*.c",
    ]),
    additional_linker_inputs = ["kernel.lds"],
    includes = ["libc/include"],
    linkopts = [
        "-nostdlib",
        "-T $(rootpath :kernel.lds)",
    ],
    deps = [
        "//lib:c",
        "//utils",
    ],
)
