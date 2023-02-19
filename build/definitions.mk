# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Figure out the current makefile directory.
define my-dir
$(strip \
	$(patsubst %/,%, \
		$(dir $(lastword $(MAKEFILE_LIST))) \
	) \
)
endef

# Find all C files under the named directory.
define all-c-files-under
$(sort
	$(patsubst ./%,%, \
		$(shell cd $(LOCAL_PATH); find -L $(1) -name "*.c" -and -not -name ".*") \
	) \
)
endef

# Find all S files under the named directory.
define all-S-files-under
$(sort
	$(patsubst ./%,%, \
		$(shell cd $(LOCAL_PATH); find -L $(1) -name "*.S" -and -not -name ".*") \
	) \
)
endef
