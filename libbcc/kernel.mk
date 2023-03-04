# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

LOCAL_PATH := $(call my-dir)

LIBBCC_SRC := $(call all-S-files-under,.)

LIBBCC_OUT := $(OUT)/$(LOCAL_PATH)
LIBBCC_AR := $(LIBBCC_OUT)/libbcc.a

LIBBCC_OBJ := $(addprefix $(LIBBCC_OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(LIBBCC_SRC)) \
	) \
)

$(LIBBCC_OUT):
	mkdir -p $@

$(LIBBCC_OBJ): | $(LIBBC_OUT)

$(LIBBCC_OUT)/%.o: $(LOCAL_PATH)/%.S | $(LIBBCC_OUT)
	$(TARGET_AS) $(TARGET_ASFLAGS) -o $@ $^

$(LIBBCC_AR): $(LIBBCC_OBJ) | $(LIBBCC_OUT)
	$(TARGET_AR) r $@ $^
