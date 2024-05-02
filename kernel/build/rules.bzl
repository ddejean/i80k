# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

# i80k_binary is essentially a cc_binary overload that helps providing the
# correct configuration to build the kernel binary.
def i80k_binary(name, **kwargs):
    native.cc_binary(
        name = name,
        # Automatically add the i80k libc as dependency.
        deps = kwargs.pop("deps", []) + [
            "//libc:c",
        ],
        # Add the application linker file to the dependencies.
        additional_linker_inputs = kwargs.pop("additional_linker_input", []) + [
            "//kernel/boards:lds",
        ],
        # Ensure the linker script is passed to the linker.
        linkopts = kwargs.pop("linkopts", []) + [
            "-T$(location //kernel/boards:lds)",
        ],
        # Copy the others arguments.
        **kwargs
    )

# i80k_library is a cc_library overload that provides the correct configuration
# to build a library for the kernel.
def i80k_library(name, **kwargs):
    native.cc_library(
        name = name,
        # Kernel libraries are always static libraries.
        linkstatic = True,
        # Kernel libraries depend on kernel libc, or nothing if ran as host
        # library.
        deps = kwargs.pop("deps", []) + select({
            "@platforms//os:none": [
                "//libc:c",
            ],
            "@platforms//os:linux": [],
        }),
        # Copy the others arguments.
        **kwargs
    )
