load(":ia16_toolchain_config.bzl", "ia16_toolchain_config")

package(default_visibility = ["//visibility:public"])

# Set of supported toolchain variants.
toolchains = {
    "8088": ":8088_toolchain",
    "8086": ":8086_toolchain",
    "186": ":186_toolchain",
    "i286": ":286_toolchain",
}

filegroup(
    name = "gcc-ia16",
    srcs = glob(["ia16-elf/**"]),
)

# Define a specific toolchain for each supported architecture.
[cc_toolchain(
    name = "{}_cc_toolchain".format(cpu),
    all_files = ":gcc-ia16",
    ar_files = ":gcc-ia16",
    compiler_files = ":gcc-ia16",
    dwp_files = ":empty",
    linker_files = ":gcc-ia16",
    objcopy_files = ":gcc-ia16",
    strip_files = ":gcc-ia16",
    supports_param_files = 0,
    toolchain_config = ":{}_cc_toolchain_config".format(cpu),
    toolchain_identifier = "{}-cc_toolchain".format(cpu),
) for cpu, name in toolchains.items()]

# Define a toolchain configuration for each supported architecture. It will help
# the toolchain config to set the correct -mtune and -march flags for the
# selected CPU.
[ia16_toolchain_config(
    name = "{}_cc_toolchain_config".format(cpu),
    cpu = cpu,
    toolchain_identifier = "{}-cc_toolchain".format(cpu),
) for cpu, name in toolchains.items()]

[toolchain(
    name = "{}_toolchain".format(cpu),
    target_compatible_with = [
        "//platforms/cpu:ia16",
        "//platforms/cpu:{}".format(cpu),
    ],
    toolchain = "{}_cc_toolchain".format(cpu),
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
) for cpu, name in toolchains.items()]
