// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _FCNTL_H_
#define _FCNTL_H_

#define O_RDONLY 0x0
#define O_WRONLY 0x1
#define O_RDWR 0x2
#define O_APPEND 0x8
#define O_CREAT 0x200
#define O_TRUNC 0x400
#define O_EXCL 0x800
#define O_SYNC 0x2000
#define O_NONBLOCK 0x4000
#define O_NOCTTY 0x8000

int open(const char *pathname, int flags, ...);

#endif  // _FCNTL_H_
