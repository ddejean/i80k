// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>
// Copyright (C) 2009 - Travis Geiselbrecht
//
// Imported from Little Kernel project (https://github.com/littlekernel/lk)
// under MIT licence.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _FS_H_
#define _FS_H_

#include <stddef.h>
#include <stdint.h>

#include "blkdev.h"

#define FS_MAX_PATH_LEN 128
#define FS_MAX_FILE_LEN 64

struct file_stat {
    bool is_dir;
    uint32_t size;
    uint32_t capacity;
};

struct fs_stat {
    uint32_t free_space;
    uint32_t total_space;

    uint32_t free_inodes;
    uint32_t total_inodes;
};

struct dirent {
    char name[FS_MAX_FILE_LEN];
};

typedef struct filehandle filehandle;
typedef struct dirhandle dirhandle;

int fs_format_device(const char *fsname, const char *device, const void *args);
int fs_mount(const char *path, const char *fs, const char *device);
int fs_unmount(const char *path);

// File API.
int fs_create_file(const char *path, filehandle **handle, uint32_t len);
int fs_open_file(const char *path, filehandle **handle);
int fs_remove_file(const char *path);
int fs_read_file(filehandle *handle, void *buf, offset_t offset, size_t len);
int fs_write_file(filehandle *handle, const void *buf, offset_t offset,
                  size_t len);
int fs_close_file(filehandle *handle);
int fs_stat_file(filehandle *handle, struct file_stat *);
int fs_truncate_file(filehandle *handle, uint32_t len);

// Directory API.
int fs_make_dir(const char *path);
int fs_open_dir(const char *path, dirhandle **handle);
int fs_read_dir(dirhandle *handle, struct dirent *ent);
int fs_close_dir(dirhandle *handle);

int fs_stat_fs(const char *mountpoint, struct fs_stat *stat);

// Helper.
int fs_load_file(const char *path, void *ptr, size_t maxlen);

// fs_normalize_path walks through a path string, removing duplicate path
// separators, flattening .  * and .. references.
void fs_normalize_path(char *path);

// Remove any leading spaces or slashes.
const char *trim_name(const char *_name);

// File system API.
typedef struct fscookie fscookie;
typedef struct filecookie filecookie;
typedef struct dircookie dircookie;

struct fs_api {
    int (*format)(struct blkdev *, const void *);
    int (*fs_stat)(fscookie *, struct fs_stat *);

    int (*mount)(struct blkdev *, fscookie **);
    int (*unmount)(fscookie *);
    int (*open)(fscookie *, const char *, filecookie **);
    int (*create)(fscookie *, const char *, filecookie **, uint32_t);
    int (*remove)(fscookie *, const char *);
    int (*truncate)(filecookie *, uint32_t);
    int (*stat)(filecookie *, struct file_stat *);
    int (*read)(filecookie *, void *, offset_t, size_t);
    int (*write)(filecookie *, const void *, offset_t, size_t);
    int (*close)(filecookie *);

    int (*mkdir)(fscookie *, const char *);
    int (*opendir)(fscookie *, const char *, dircookie **);
    int (*readdir)(dircookie *, struct dirent *);
    int (*closedir)(dircookie *);
};

struct fs {
    const char *name;
    const struct fs_api *api;
};

// Define in your fs implementation to register your api with the fs layer.
#define FS(_name, _api)                                                   \
    const struct fs __fs_impl_##_name __attribute__((section(".fs"))) = { \
        .name = #_name,                                                   \
        .api = _api,                                                      \
    }

// List all registered file systems.
void fs_dump_list(void);

// List all mount points.
void fs_dump_mounts(void);

#endif  // _FS_H_
