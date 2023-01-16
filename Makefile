# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

SOURCES := \
	crt0.S \
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

$(OUT):
	mkdir -p $@

$(OUT)/%.o: %.S | $(OUT)
	nasm -f as86 -o $@ $^

$(OUT)/%.o: %.c | $(OUT)
	bcc -0 -ansi -o $@ -c $^

$(KERNEL_BIN): $(OBJECTS)
	ld86 -d -T 0x8000 -D 0x0 -m -M -o $@ $^

$(BOOTSTRAP_BIN): bootstrap.S
	nasm -f bin -o $@ $^

$(EEPROM_BIN): $(KERNEL_BIN) $(BOOTSTRAP_BIN)
	dd if=/dev/zero ibs=1k count=32 | tr "\000" "\377" > $@
	dd if=$(KERNEL_BIN) of=$@ conv=notrunc
	dd if=$(BOOTSTRAP_BIN) of=$@ bs=1 seek=32752 conv=notrunc

.PHONY: all
all: $(EEPROM_BIN)

.PHONY: clean
clean:
	rm -rf $(OUT)
