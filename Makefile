# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Include build helpers.
include build/definitions.mk
include build/toolchains.mk

SOURCES := \
	crt0.S \
	int.S \
	mem.S \
	cpu.c \
	interrupts.c \
	firmware.c \
	heap.c \
	kernel.c

OUT := out

OBJECTS := $(addprefix $(OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(SOURCES)) \
	) \
)

KERNEL_BIN := $(OUT)/kernel.bin
BOOTSTRAP_BIN := $(OUT)/bootstrap.bin
EEPROM_BIN := $(OUT)/eeprom.bin

# Include librairies dependencies.
include libc/kernel.mk
# Include tests build rules.
include tests/kernel.mk

$(OUT):
	mkdir -p $@

$(OUT)/%.o: %.S | $(OUT)
	$(TARGET_AS) $(TARGET_ASFLAGS) -o $@ $^

$(OUT)/%.o: %.c | $(OUT)
	$(TARGET_CC) $(TARGET_CLFAGS) -Ilibc/include -o $@ -c $^

$(KERNEL_BIN): $(OBJECTS) $(LIBC_AR)
	$(TARGET_LD) -d -i -T 0x8000 -D 0x400 -m -M -o $@ $^

$(BOOTSTRAP_BIN): bootstrap.S
	$(TARGET_AS) -f bin -o $@ $^

.DEFAULT_GOAL := $(EEPROM_BIN)
$(EEPROM_BIN): $(KERNEL_BIN) $(BOOTSTRAP_BIN)
	dd if=/dev/zero ibs=1k count=32 | tr "\000" "\377" > $@
	dd if=$(KERNEL_BIN) of=$@ conv=notrunc
	dd if=$(BOOTSTRAP_BIN) of=$@ bs=1 seek=32752 conv=notrunc

.PHONY: clean
clean:
	rm -rf $(OUT)
