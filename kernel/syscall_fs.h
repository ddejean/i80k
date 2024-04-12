// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SYSCALL_FS_H_
#define _SYSCALL_FS_H_

int sys_open(const char *pathname, int flags, int mode);
ssize_t sys_read(int fd, void *buf, size_t count);
ssize_t sys_write(int fd, const void *buf, size_t count);
int sys_close(int fd);

#endif  // _SYSCALL_FS_H_
