// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <stdint.h>
#include <sys/types.h>

// brk sets the end of the data segment to the value specified by addr.
// On success brk() returns 0. On error -1 is returned.
int brk(void *addr);

// sbrk() increments the program's data space by increment bytes. Calling sbrk()
// with an increment of 0 can be used to find the current location of the
// program break.
// On success, sbrk() returns the previous program break.  (If the break was
// increased, then this value is a pointer to the start of the newly allocated
// memory).  On error, (void *) -1 is returned.
void *sbrk(intptr_t increment);

// getpid returns the process ID (PID) of the calling process.
pid_t getpid(void);

// read attempts to read up to |count| bytes from file description |fd| into
// buffer starting at |buf|.
ssize_t read(int fd, void *buf, size_t count);

// write writes up to |count|bytes from the buffer starting at |buf| to the file
// referred to by the file descriptor |fd|.
ssize_t write(int fd, const void *buf, size_t count);

// close closes a file descriptor, so that it no longer refers to any file and
// may be reused.
int close(int fd);

#endif  // _UNISTD_H_