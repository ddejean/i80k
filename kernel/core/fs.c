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

#include "fs.h"

#include <bsd/string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "blkdev.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct fs_mount {
    struct list_node node;

    char *path;
    size_t pathlen;  // save the strlen of path above to help with path matching
    struct blkdev *dev;
    fscookie *cookie;
    int ref;
    const struct fs *fs;
    const struct fs_api *api;
};

struct filehandle {
    filecookie *cookie;
    struct fs_mount *mount;
};

struct dirhandle {
    dircookie *cookie;
    struct fs_mount *mount;
};

static struct list_node mounts = LIST_INITIAL_VALUE(mounts);
static struct list_node fses = LIST_INITIAL_VALUE(fses);

// defined by the linker, wrapping all structs in the "fs" section
extern const struct fs _fs_start[];
extern const struct fs _fs_end[];

static const struct fs *find_fs(const char *name) {
    for (const struct fs *fs = _fs_start; fs != _fs_end; fs++) {
        if (!strcmp(name, fs->name)) return fs;
    }
    return NULL;
}

void fs_dump_list(void) {
    for (const struct fs *fs = _fs_start; fs != _fs_end; fs++) {
        puts(fs->name);
    }
}

void fs_dump_mounts(void) {
    printf("%-16s%s\n", "Filesystem", "Path");
    struct fs_mount *mount;
    list_for_every_entry(&mounts, mount, struct fs_mount, node) {
        printf("%-16s%s\n", mount->fs->name, mount->path);
    }
}

// find a mount structure based on the prefix of this path
// bump the ref to the mount structure before returning
static struct fs_mount *find_mount(const char *path,
                                   const char **trimmed_path) {
    // paths must be absolute and start with /
    if (path[0] != '/') {
        return NULL;
    }
    size_t pathlen = strlen(path);

    struct fs_mount *mount;
    list_for_every_entry(&mounts, mount, struct fs_mount, node) {
        // if the path is shorter than this mount point, no point continuing
        if (pathlen < mount->pathlen) {
            continue;
        }

        if (memcmp(path, mount->path, mount->pathlen) == 0) {
            // If we got a match, make sure the next element in the path is
            // a path separator or the end of the string. This keeps from
            // matching /foo2 with /foo, but /foo/bar would match correctly.
            if (path[mount->pathlen] != '/' && path[mount->pathlen] != 0) {
                continue;
            }

            // we got a match, skip forward to the next element
            if (trimmed_path) {
                *trimmed_path = &path[mount->pathlen];
                // if we matched against the end of the path, at least return
                // a "/".
                // TODO: decide if this is necessary
                if (*trimmed_path[0] == 0) {
                    *trimmed_path = "/";
                }
            }

            mount->ref++;

            return mount;
        }
    }

    return NULL;
}

// decrement the ref to the mount structure, which may
// cause an unmount operation
static void put_mount(struct fs_mount *mount) {
    if ((--mount->ref) == 0) {
        list_delete(&mount->node);
        mount->api->unmount(mount->cookie);
        free(mount->path);
        free(mount);
    }
}

static int mount(const char *path, const char *device, const struct fs *fs) {
    struct fs_mount *mount;
    const struct fs_api *api = fs->api;
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    if (temppath[0] != '/') return -1;

    /* see if there's already something at this path, abort if there is */
    mount = find_mount(temppath, NULL);
    if (mount) {
        put_mount(mount);
        return ERR_INVAL;
    }

    /* open a bio device if the string is nonnull */
    struct blkdev *dev = NULL;
    if (device && device[0] != '\0') {
        dev = blk_open(device);
        if (!dev) return ERR_NO_DEV;
    }

    /* call into the fs implementation */
    fscookie *cookie;
    int err = api->mount(dev, &cookie);
    if (err < 0) {
        return err;
    }

    /* create the mount structure and add it to the list */
    mount = malloc(sizeof(struct fs_mount));
    if (!mount) {
        return ERR_NO_MEM;
    }
    mount->path = strdup(temppath);
    if (!mount->path) {
        free(mount);
        return ERR_NO_MEM;
    }
    mount->pathlen = strlen(mount->path);
    mount->dev = dev;
    mount->cookie = cookie;
    mount->ref = 1;
    mount->fs = fs;
    mount->api = api;

    list_add_head(&mounts, &mount->node);

    return 0;
}

int fs_format_device(const char *fsname, const char *device, const void *args) {
    const struct fs *fs = find_fs(fsname);
    if (!fs) {
        return ERR_INVAL;
    }

    if (fs->api->format == NULL) {
        return ERR_INVAL;
    }

    struct blkdev *dev = NULL;
    if (device && device[0] != '\0') {
        dev = blk_open(device);
        if (!dev) return ERR_NO_DEV;
    }

    return fs->api->format(dev, args);
}

int fs_mount(const char *path, const char *fsname, const char *device) {
    const struct fs *fs = find_fs(fsname);
    if (!fs) return ERR_NO_ENTRY;

    return mount(path, device, fs);
}

int fs_unmount(const char *path) {
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    struct fs_mount *mount = find_mount(temppath, NULL);
    if (!mount) return ERR_NO_ENTRY;

    // return the ref that find_mount added and one extra
    put_mount(mount);
    put_mount(mount);

    return 0;
}

int fs_open_file(const char *path, filehandle **handle) {
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    const char *newpath;
    struct fs_mount *mount = find_mount(temppath, &newpath);
    if (!mount) return ERR_NO_ENTRY;

    filecookie *cookie;
    int err = mount->api->open(mount->cookie, newpath, &cookie);
    if (err < 0) {
        put_mount(mount);
        return err;
    }

    filehandle *f = malloc(sizeof(*f));
    f->cookie = cookie;
    f->mount = mount;
    *handle = f;

    return 0;
}

int fs_create_file(const char *path, filehandle **handle, uint32_t len) {
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    const char *newpath;
    struct fs_mount *mount = find_mount(temppath, &newpath);
    if (!mount) return ERR_NO_ENTRY;

    if (!mount->api->create) {
        put_mount(mount);
        return -1;
    }

    filecookie *cookie;
    int err = mount->api->create(mount->cookie, newpath, &cookie, len);
    if (err < 0) {
        put_mount(mount);
        return err;
    }

    filehandle *f = malloc(sizeof(*f));
    if (!f) {
        put_mount(mount);
        return err;
    }
    f->cookie = cookie;
    f->mount = mount;
    *handle = f;

    return 0;
}

int fs_truncate_file(filehandle *handle, uint32_t len) {
    if (!handle) return ERR_INVAL;

    return handle->mount->api->truncate(handle->cookie, len);
}

int fs_remove_file(const char *path) {
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    const char *newpath;
    struct fs_mount *mount = find_mount(temppath, &newpath);
    if (!mount) return ERR_NO_ENTRY;

    if (!mount->api->remove) {
        put_mount(mount);
        return ERR_NOT_SUPP;
    }

    int err = mount->api->remove(mount->cookie, newpath);

    put_mount(mount);

    return err;
}

int fs_read_file(filehandle *handle, void *buf, off_t offset, size_t len) {
    return handle->mount->api->read(handle->cookie, buf, offset, len);
}

int fs_write_file(filehandle *handle, const void *buf, off_t offset,
                  size_t len) {
    if (!handle->mount->api->write) return -1;

    return handle->mount->api->write(handle->cookie, buf, offset, len);
}

int fs_close_file(filehandle *handle) {
    int err = handle->mount->api->close(handle->cookie);
    if (err < 0) return err;

    put_mount(handle->mount);
    free(handle);
    return 0;
}

int fs_stat_file(filehandle *handle, struct file_stat *stat) {
    return handle->mount->api->stat(handle->cookie, stat);
}

int fs_make_dir(const char *path) {
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    const char *newpath;
    struct fs_mount *mount = find_mount(temppath, &newpath);
    if (!mount) return ERR_NO_ENTRY;

    if (!mount->api->mkdir) {
        put_mount(mount);
        return ERR_NOT_SUPP;
    }

    int err = mount->api->mkdir(mount->cookie, newpath);

    put_mount(mount);

    return err;
}

int fs_open_dir(const char *path, dirhandle **handle) {
    char temppath[FS_MAX_PATH_LEN];

    strlcpy(temppath, path, sizeof(temppath));
    fs_normalize_path(temppath);

    const char *newpath;
    struct fs_mount *mount = find_mount(temppath, &newpath);
    if (!mount) return ERR_NO_ENTRY;

    if (!mount->api->opendir) {
        put_mount(mount);
        return ERR_NOT_SUPP;
    }

    dircookie *cookie;
    int err = mount->api->opendir(mount->cookie, newpath, &cookie);
    if (err < 0) {
        put_mount(mount);
        return err;
    }

    dirhandle *d = malloc(sizeof(*d));
    if (!d) {
        put_mount(mount);
        return ERR_NO_MEM;
    }
    d->cookie = cookie;
    d->mount = mount;
    *handle = d;

    return 0;
}

int fs_read_dir(dirhandle *handle, struct dirent *ent) {
    if (!handle->mount->api->readdir) return ERR_NOT_SUPP;

    return handle->mount->api->readdir(handle->cookie, ent);
}

int fs_close_dir(dirhandle *handle) {
    if (!handle->mount->api->closedir) return ERR_NOT_SUPP;

    int err = handle->mount->api->closedir(handle->cookie);
    if (err < 0) return err;

    put_mount(handle->mount);
    free(handle);
    return 0;
}

int fs_stat_fs(const char *mountpoint, struct fs_stat *stat) {
    if (!stat) {
        return ERR_INVAL;
    }

    const char *newpath;
    struct fs_mount *mount = find_mount(mountpoint, &newpath);
    if (!mount) {
        return ERR_NO_ENTRY;
    }

    if (!mount->api->fs_stat) {
        put_mount(mount);
        return ERR_NOT_SUPP;
    }

    int result = mount->api->fs_stat(mount->cookie, stat);

    put_mount(mount);

    return result;
}

int fs_load_file(const char *path, void *ptr, size_t maxlen) {
    filehandle *handle;

    /* open the file */
    int err = fs_open_file(path, &handle);
    if (err < 0) return err;

    /* stat it for size, see how much we need to read */
    struct file_stat stat;
    fs_stat_file(handle, &stat);

    int read_bytes = fs_read_file(handle, ptr, 0, MIN(maxlen, stat.size));

    fs_close_file(handle);

    return read_bytes;
}

const char *trim_name(const char *_name) {
    const char *name = &_name[0];
    // chew up leading spaces
    while (*name == ' ') name++;

    // chew up leading slashes
    while (*name == '/') name++;

    return name;
}

void fs_normalize_path(char *path) {
    int outpos;
    int pos;
    char c;
    bool done;
    enum {
        INITIAL,
        FIELD_START,
        IN_FIELD,
        SEP,
        SEEN_SEP,
        DOT,
        SEEN_DOT,
        DOTDOT,
        SEEN_DOTDOT,
    } state;

    state = INITIAL;
    pos = 0;
    outpos = 0;
    done = false;

    // Remove duplicate path separators, flatten empty fields (only composed of
    // * .), backtrack fields with .., remove trailing slashes.
    while (!done) {
        c = path[pos];
        switch (state) {
            case INITIAL:
                if (c == '/') {
                    state = SEP;
                } else if (c == '.') {
                    state = DOT;
                } else {
                    state = FIELD_START;
                }
                break;
            case FIELD_START:
                if (c == '.') {
                    state = DOT;
                } else if (c == 0) {
                    done = true;
                } else {
                    state = IN_FIELD;
                }
                break;
            case IN_FIELD:
                if (c == '/') {
                    state = SEP;
                } else if (c == 0) {
                    done = true;
                } else {
                    path[outpos++] = c;
                    pos++;
                }
                break;
            case SEP:
                pos++;
                path[outpos++] = '/';
                state = SEEN_SEP;
                break;
            case SEEN_SEP:
                if (c == '/') {
                    // eat it
                    pos++;
                } else if (c == 0) {
                    done = true;
                } else {
                    state = FIELD_START;
                }
                break;
            case DOT:
                pos++;  // consume the dot
                state = SEEN_DOT;
                break;
            case SEEN_DOT:
                if (c == '.') {
                    // dotdot now
                    state = DOTDOT;
                } else if (c == '/') {
                    // a field composed entirely of a .
                    // consume the / and move directly to the SEEN_SEP state
                    pos++;
                    state = SEEN_SEP;
                } else if (c == 0) {
                    done = true;
                } else {
                    // a field prefixed with a .
                    // emit a . and move directly into the IN_FIELD state
                    path[outpos++] = '.';
                    state = IN_FIELD;
                }
                break;
            case DOTDOT:
                pos++;  // consume the dot
                state = SEEN_DOTDOT;
                break;
            case SEEN_DOTDOT:
                if (c == '/' || c == 0) {
                    // a field composed entirely of '..'
                    // search back and consume a field we've already emitted
                    if (outpos > 0) {
                        // we have already consumed at least one field
                        outpos--;

                        // walk backwards until we find the next field boundary
                        while (outpos > 0) {
                            if (path[outpos - 1] == '/') {
                                break;
                            }
                            outpos--;
                        }
                    }
                    pos++;
                    state = SEEN_SEP;
                    if (c == 0) done = true;
                } else {
                    // a field prefixed with ..
                    // emit the .. and move directly to the IN_FIELD state
                    path[outpos++] = '.';
                    path[outpos++] = '.';
                    state = IN_FIELD;
                }
                break;
        }
    }

    /* don't end with trailing slashes */
    if (outpos > 0 && path[outpos - 1] == '/') outpos--;

    path[outpos++] = 0;
}
