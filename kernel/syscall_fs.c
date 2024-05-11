// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <sys/types.h>

#include "error.h"
#include "fs.h"
#include "scheduler.h"
#include "task.h"

int sys_open(const char *pathname, int flags, int mode) {
    int ret;
    filehandle *fh;

    // flags and mode are ignored for now.
    (void)flags;
    (void)mode;

    ret = fs_open_file(pathname, &fh);
    if (ret < 0) {
        return ret;
    }

    ret = task_put_desc(scheduler_current(), DESC_FILE, fh);
    if (ret < 0) {
        fs_close_file(fh);
        return ret;
    }

    return ret;
}

ssize_t sys_read(int fd, void *buf, size_t count) {
    struct task *current;
    const struct descriptor *desc;

    current = scheduler_current();
    desc = task_get_desc(current, fd);
    if (!desc) {
        return ERR_INVAL;
    }
    if (desc->type != DESC_FILE) {
        return ERR_NOT_SUPP;
    }
    // TODO: handle offsets.
    return fs_read_file(desc->handle.file, buf, 0, count);
}

ssize_t sys_write(int fd, const void *buf, size_t count) {
    struct task *current;
    const struct descriptor *desc;

    current = scheduler_current();
    desc = task_get_desc(current, fd);
    if (!desc) {
        return ERR_INVAL;
    }
    if (desc->type != DESC_FILE) {
        return ERR_NOT_SUPP;
    }
    // TODO: handle offsets.
    return fs_write_file(desc->handle.file, buf, 0, count);
}

int sys_close(int fd) {
    struct task *current;
    const struct descriptor *desc;

    current = scheduler_current();
    desc = task_get_desc(current, fd);
    if (!desc) {
        return ERR_INVAL;
    }
    switch (desc->type) {
        case DESC_FILE:
            fs_close_file(desc->handle.file);
            break;
        case DESC_DIR:
            fs_close_dir(desc->handle.dir);
            break;
        default:
            return ERR_NOT_SUPP;
    }
    task_reset_desc(current, fd);
    return 0;
}

int sys_mount(const char *source, const char *target,
              const char *filesystemtype) {
    return fs_mount(target, filesystemtype, source);
}