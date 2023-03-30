# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Include build helpers.
include build/definitions.mk
include build/toolchains.mk

SOURCES := \
	crt0.S \
	bootstrap.S \
	mem.S \
	sched.S \
	int.S \
	console.c \
	interrupts.c \
	firmware.c \
	heap.c \
	hwalloc.c \
	scheduler.c \
	uart.c \
	ringbuffer.c \
	kernel.c

OUT := out

OBJECTS := $(addprefix $(OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(SOURCES)) \
	) \
)
KERNEL_BIN := $(OUT)/kernel.bin

# Include librairies dependencies.
include libc/kernel.mk
# Include tests build rules.
include tests/kernel.mk

$(OUT):
	mkdir -p $@

$(OUT)/%.o: %.S | $(OUT)
	$(TARGET_AS) $(TARGET_ASFLAGS) -o $@ $^

$(OUT)/%.o: %.c | $(OUT)
	$(TARGET_CC) $(TARGET_CLFAGS) -o $@ -c $^

.DEFAULT_GOAL := $(KERNEL_BIN)
$(KERNEL_BIN): kernel.lds $(OBJECTS) $(LIBC_AR)
	$(TARGET_LD) $(TARGET_LDFLAGS) -T $< -o $@ $(filter-out $<, $^) $(TARGET_LDLIBS)

.PHONY: clean
clean:
	rm -rf $(OUT)
