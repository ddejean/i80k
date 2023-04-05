load(":ia16_toolchain_config.bzl", "ia16_toolchain_config")

package(default_visibility = ["//visibility:public"])

# Set of supported toolchain variants.
toolchains = {
    "i8088": ":i8088_toolchain",
    "i8086": ":i8086_toolchain",
    "i186": ":i186_toolchain",
    "i286": ":i286_toolchain",
}

cc_toolchain_suite(
    name = "ia16_suite",
    toolchains = toolchains,
)

filegroup(
    name = "gcc-ia16",
    srcs = glob(["ia16-elf/**"]),
)

# Define a specific toolchain for each supported architecture.
[cc_toolchain(
    name = "{}_toolchain".format(cpu),
    all_files = ":gcc-ia16",
    ar_files = ":gcc-ia16",
    compiler_files = ":gcc-ia16",
    dwp_files = ":empty",
    linker_files = ":gcc-ia16",
    objcopy_files = ":gcc-ia16",
    strip_files = ":gcc-ia16",
    supports_param_files = 0,
    toolchain_config = ":{}_toolchain_config".format(cpu),
    toolchain_identifier = "{}-toolchain".format(cpu),
) for cpu, name in toolchains.items()]

# Define a toolchain configuration for each supported architecture. It will help
# the toolchain config to set the correct -mtune and -march flags for the
# selected CPU.
[ia16_toolchain_config(
    name = "{}_toolchain_config".format(cpu),
    cpu = cpu,
    toolchain_identifier = "{}-toolchain".format(cpu),
) for cpu, name in toolchains.items()]
