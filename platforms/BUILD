package(default_visibility = ["//visibility:public"])

constraint_value(
    name = "ia16",
    constraint_setting = "@platforms//cpu:cpu",
    visibility = ["//visibility:public"],
)

constraint_setting(name = "cpu_variant")

constraint_value(
    name = "8088",
    constraint_setting = ":cpu_variant",
    visibility = ["//visibility:public"],
)

constraint_value(
    name = "8086",
    constraint_setting = ":cpu_variant",
    visibility = ["//visibility:public"],
)

constraint_value(
    name = "80186",
    constraint_setting = ":cpu_variant",
    visibility = ["//visibility:public"],
)

constraint_value(
    name = "80286",
    constraint_setting = ":cpu_variant",
    visibility = ["//visibility:public"],
)

platform(
    name = "none_8088",
    constraint_values = [
        "@platforms//os:none",
        ":ia16",
        ":8088",
    ],
)
