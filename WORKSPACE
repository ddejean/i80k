# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Register the toolchain(s) declared in //toolchains.
register_toolchains("//toolchains:8088_toolchain")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Gtest dependency.
http_archive(
    name = "com_google_googletest",
    sha256 = "755f9a39bc7205f5a0c428e920ddad092c33c8a1b46997def3f1d4a82aded6e1",
    strip_prefix = "googletest-5ab508a01f9eb089207ee87fd547d290da39d015",
    urls = ["https://github.com/google/googletest/archive/5ab508a01f9eb089207ee87fd547d290da39d015.zip"],
)
