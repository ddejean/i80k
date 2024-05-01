# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

def i80k_binary(name, **kwargs):
    native.cc_binary(
        name = name,
        # Automatically add the i80k libc as dependency.
        deps = kwargs.pop("deps", []) + [
            "//libc:c",
        ],
        # Add the application linker file to the dependencies.
        additional_linker_inputs = kwargs.pop("additional_linker_input", []) + [
            "//apps/build:lds",
        ],
        # Ensure the linker script is passed to the linker.
        linkopts = kwargs.pop("linkopts", []) + [
            "-T$(location //apps/build:lds)",
        ],
        # Do not link to standard libraries if any.
        features = kwargs.pop("features", []) + [
            "no_stdlib",
        ],
        **kwargs
    )
