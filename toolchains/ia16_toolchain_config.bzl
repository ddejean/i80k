# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

load(
    "@rules_cc//cc:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
)
load("@rules_cc//cc:action_names.bzl", "ACTION_NAME_GROUPS")

def _impl(ctx):
    # Compiler tools paths.
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "ia16-elf/bin/ia16-elf-gcc",
        ),
        tool_path(
            name = "ld",
            path = "ia16-elf/bin/ia16-elf-ld",
        ),
        tool_path(
            name = "ar",
            path = "ia16-elf/bin/ia16-elf-ar",
        ),
        tool_path(
            name = "cpp",
            path = "ia16-elf/bin/ia16-elf-cpp",
        ),
        tool_path(
            name = "gcov",
            path = "ia16-elf/bin/ia16-elf-gcov",
        ),
        tool_path(
            name = "nm",
            path = "ia16-elf/bin/ia16-elf-nm",
        ),
        tool_path(
            name = "objdump",
            path = "ia16-elf/bin/ia16-elf-objdump",
        ),
        tool_path(
            name = "strip",
            path = "ia16-elf/bin/ia16-elf-strip",
        ),
    ]

    # Add the -no-canonical-prefixes flag to all compilation steps to ensure GCC
    # is not using an absolute path for the system headers.
    no_canonical_prefixes_feature = feature(
        name = "no_canonical_prefixes",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = [
                            "-no-canonical-prefixes",
                        ],
                    ),
                ],
            ),
        ],
    )

    # Ensure the CPU specific flags will be set.
    arch_compile_flags_feature = feature(
        name = "arch_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = [
                            "-march=i{}".format(ctx.attr.cpu),
                            "-mtune=i{}".format(ctx.attr.cpu),
                        ],
                    ),
                ],
            ),
        ],
    )

    treat_warnings_as_errors_feature = feature(
        name = "treat_warnings_as_errors",
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(flags = ["-Werror"]),
                ],
            ),
        ],
    )

    all_warnings_feature = feature(
        name = "all_warnings",
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(flags = ["-Wall"]),
                ],
            ),
        ],
    )

    extra_warnings_feature = feature(
        name = "extra_warnings",
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(flags = ["-Wextra"]),
                ],
            ),
        ],
    )

    # Tell the linker where to find GCC builtins library.
    default_linker_flags_feature = feature(
        name = "default_linker_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_link_actions,
                flag_groups = [
                    flag_group(flags = [
                        "-L",
                        "toolchain/ia16-elf/lib/gcc/ia16-elf/6.3.0",
                        "-lgcc",
                    ]),
                ],
            ),
        ],
    )

    no_stdlib_feature = feature(
        name = "no_stdlib",
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_link_actions,
                flag_groups = [
                    flag_group(flags = ["-nostdlib"]),
                ],
            ),
        ],
    )

    tiny_memory_model_feature = feature(
        name = "tiny_memory_model",
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(flags = ["-mcmodel=tiny"]),
                ],
            ),
        ],
    )

    small_memory_model_feature = feature(
        name = "small_memory_model",
        flag_sets = [
            flag_set(
                actions = ACTION_NAME_GROUPS.all_cc_compile_actions,
                flag_groups = [
                    flag_group(flags = ["-mcmodel=small"]),
                ],
            ),
        ],
    )

    # Compiler includes.
    cxx_builtin_include_directories = [
        "toolchains/ia16-elf/lib/gcc/ia16-elf/6.3.0/include",
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        cxx_builtin_include_directories = cxx_builtin_include_directories,
        features = [
            no_canonical_prefixes_feature,
            arch_compile_flags_feature,
            treat_warnings_as_errors_feature,
            all_warnings_feature,
            extra_warnings_feature,
            default_linker_flags_feature,
            no_stdlib_feature,
            tiny_memory_model_feature,
            small_memory_model_feature,
        ],
        toolchain_identifier = ctx.attr.toolchain_identifier,
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "ia16",
        target_libc = "unknown",
        compiler = "gcc-ia16-6.3.0",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
    )

ia16_toolchain_config = rule(
    implementation = _impl,
    attrs = {
        "toolchain_identifier": attr.string(mandatory = True),
        "cpu": attr.string(default = "any", mandatory = True),
    },
    provides = [CcToolchainConfigInfo],
)
