# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

LOCAL_PATH := $(call my-dir)

LIBC_SRC := \
	$(call all-S-files-under,.) \
	$(call all-c-files-under,.)

LIBC_OUT := $(OUT)/$(LOCAL_PATH)
LIBC_AR := $(LIBC_OUT)/libc.a

LIBC_OBJ := $(addprefix $(LIBC_OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(LIBC_SRC)) \
	) \
)

$(LIBC_OUT):
	mkdir -p $@

$(LIBC_OBJ): | $(LIBC_OUT)

$(LIBC_OUT)/%.o: $(LOCAL_PATH)/%.c
	$(TARGET_CC) $(TARGET_CLFAGS) -Ilibc/include -o $@ -c $^

$(LIBC_OUT)/%.o: $(LOCAL_PATH)/%.S
	$(TARGET_AS) $(TARGET_ASFLAGS) -o $@ $^

$(LIBC_AR): $(LIBC_OBJ) | $(LIBC_OUT)
	$(TARGET_AR) r $@ $^
