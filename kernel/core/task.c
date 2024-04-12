// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "task.h"

#include <stdlib.h>

#include "error.h"

void task_set_desc(struct descriptor *d, desc_t type, void *handle) {
    d->type = type;
    switch (type) {
        case DESC_FILE:
            d->handle.file = handle;
            break;
        case DESC_DIR:
            d->handle.dir = handle;
            break;
        default:
            // TODO: throw an error.
            return;
    }
    return;
}

int task_put_desc(struct task *t, desc_t type, void *handle) {
    if (!t->descriptors) {
        // Table is not allocated yet.
        t->descriptors = calloc(MAX_FD, sizeof(desc_t));
    }
    for (int i = 0; i < MAX_FD; i++) {
        if (t->descriptors[i].type == DESC_UNSET) {
            task_set_desc(&(t->descriptors[i]), type, handle);
            return i;
        }
    }
    // No space left in the descriptor table, return an error.
    return ERR_NO_MEM;
}

const struct descriptor *task_get_desc(struct task *t, int fd) {
    if (!t->descriptors) {
        return NULL;
    }
    if (t->descriptors[fd].type != DESC_UNSET) {
        return (const struct descriptor *)(&t->descriptors[fd]);
    }
    return NULL;
}

void task_reset_desc(struct task *t, int fd) {
    t->descriptors[fd].type = DESC_UNSET;
}