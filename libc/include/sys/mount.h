// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _MOUNT_H_
#define _MOUNT_H_

int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data);

#endif  // _MOUNT_H_
