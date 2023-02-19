# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

LOCAL_PATH := $(call my-dir)

TESTS_OUT := $(OUT)/tests
TESTS_SRC := $(call all-c-files-under,.)
TESTS_OBJ :=  $(addprefix $(TESTS_OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(TESTS_SRC)) \
	) \
)

TESTS_KERNEL_OUT := $(TESTS_OUT)/kernel
TESTS_KERNEL_SRC := \
	hwalloc.c
TESTS_KERNEL_OBJ := $(addprefix $(TESTS_KERNEL_OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(TESTS_KERNEL_SRC)) \
	) \
)

# Common rules to build test object files.
$(TESTS_OUT) $(TESTS_KERNEL_OUT):
	mkdir -p $@

$(TESTS_OUT)/%.o: $(LOCAL_PATH)/%.c | $(TESTS_OUT)
	$(HOST_CC) -I. -c $^ -o $@

$(TESTS_KERNEL_OUT)/%.o: %.c | $(TESTS_KERNEL_OUT)
	$(HOST_CC) -c $^ -o $@

$(TESTS_OUT)/tests: $(TESTS_OBJ) $(TESTS_KERNEL_OBJ)
	$(HOST_LD) -lcunit -o $@ $^

tests: $(TESTS_OUT)/tests
	$(TESTS_OUT)/tests
