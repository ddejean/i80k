# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

load("@aspect_bazel_lib//lib:copy_to_directory.bzl", "copy_to_directory_bin_action")

def _ext2_image_impl(ctx):
    # Create the temporary image directory.
    dir = ctx.actions.declare_directory(ctx.label.name + ".tmp")

    # Create the list of files to copy.
    files_to_copy = []
    for d in ctx.attr.deps:
        files_to_copy.extend(d.files.to_list())

    # Find the copy_to_directory tool.
    _copy_to_directory_bin = ctx.toolchains["@aspect_bazel_lib//lib:copy_to_directory_toolchain_type"].copy_to_directory_info.bin

    # Copy all the files the temporary directory.
    copy_to_directory_bin_action(
        ctx = ctx,
        name = ctx.label.name + "CopyToDirectory",
        dst = dir,
        files = files_to_copy,
        copy_to_directory_bin = _copy_to_directory_bin,
    )

    # Declare the output image file.
    output_file = ctx.actions.declare_file(ctx.label.name)

    # Build mke2fs arguments.
    args = ctx.actions.args()

    # Run quietly
    args.add("-q")

    # Ext2 filesystem.
    args.add("-t", "ext2")

    # Add filesystem options if any.
    args.add_joined("-O", ctx.attr.options, join_with = ",")

    # Directory containing the files to add to the image.
    args.add("-d", dir.path)

    # Image size.
    args.add("-b", ctx.attr.block_size)

    # Image output file.
    args.add(output_file.path)

    args.add(ctx.attr.fs_size)

    ctx.actions.run(
        mnemonic = "MakeExt2Fs",
        inputs = [dir],
        outputs = [output_file],
        executable = "/sbin/mke2fs",
        arguments = [args],
        progress_message = "building %{output} ext2 image",
    )

    return [DefaultInfo(files = depset([output_file]))]

_attrs = {
    "deps": attr.label_list(
        doc = "List of dependencies to include in the image",
    ),
    "options": attr.string_list(
        doc = "List of options to configure the filesystem",
    ),
    "block_size": attr.int(
        doc = "Size of a block in bytes",
    ),
    "fs_size": attr.int(
        doc = "Size of the image or number of blocks in the image",
        mandatory = True,
    ),
}

ext2_image = rule(
    implementation = _ext2_image_impl,
    attrs = _attrs,
    toolchains = [
        Label("@aspect_bazel_lib//lib:copy_to_directory_toolchain_type"),
    ],
)
