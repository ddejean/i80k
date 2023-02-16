# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

LIBC_SRC := strings.S
LIBC_OUT := $(OUT)/libc
LIBC_AR := $(LIBC_OUT)/libc.a

LIBC_OBJ := $(addprefix $(LIBC_OUT)/, \
	$(patsubst %.S,%.o, \
		$(patsubst %.c,%.o,$(LIBC_SRC)) \
	) \
)

$(LIBC_OUT):
	mkdir -p $@

$(LIBC_OBJ): | $(LIBC_OUT)

$(LIBC_OUT)/%.o: libc/%.c
	$(CC) $(CLFAGS) -o $@ -c $^

$(LIBC_AR): $(LIBC_OBJ) | $(LIBC_OUT)
	$(AR) r $@ $^
