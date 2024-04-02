// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef long int int32_t;
typedef unsigned long int uint32_t;
typedef int int16_t;
typedef unsigned int uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

// System V compatibility
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// Used for file size.
typedef long off_t;

// Used as a general identifier; can be used to contain at list pid_t, uid_t or
// gid_t.
typedef int id_t;

// Used for process IDs and process group IDs.
typedef int pid_t;

// Used for a count of bytes or an error indication.
typedef int ssize_t;

typedef unsigned long clock_t;
typedef long long time_t;
typedef long long suseconds_t;

#endif  // _SYS_TYPES_H_
