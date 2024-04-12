// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _TASK_H_
#define _TASK_H_

#include <sys/types.h>

#include "ctx.h"
#include "fs.h"
#include "list.h"

// Process state.
enum task_state {
    READY,
    RUNNING,
    WAITING,
    ZOMBIE,
};

// File descriptor types.
typedef enum {
    DESC_UNSET = 0,
    DESC_FILE,  // Descriptor for a file.
    DESC_DIR,   // Descriptor for a directory.

    DESC_MAX,
} desc_t;

// Internal representation of a file descriptor.
struct descriptor {
    // Type of the file descriptor.
    desc_t type;
    union {
        filehandle *file;
        dirhandle *dir;
    } handle;
};

// Maximum number of file descriptors.
#define MAX_FD 16

// Represents a process.
struct task {
    // List node of the processes list.
    struct list_node node;

    // Process state.
    enum task_state state;
    // Process ID.
    pid_t pid;
    // Parent process ID.
    pid_t parent;
    // Process priority.
    int prio;
    // Process context.
    struct context ctx;
    // Process return value.
    int status;

    // Process stack.
    void *stack;

    // Private data use for wait condition.
    void *wait_state;

    // Table of file descriptors associated to this task.
    struct descriptor *descriptors;
};

#define WAIT_STATE(t, type) ((type *)t->wait_state)

// task_put_fd adds |handle| of |type| in |t| descriptors table.
int task_put_desc(struct task *t, desc_t type, void *handle);

// task_get_desc returns the descriptor associated with |fd| if it exists, NULL
// otherwise.
const struct descriptor *task_get_desc(struct task *t, int fd);

// task_reset_desc resets the descriptor associated with |fd|.
void task_reset_desc(struct task *t, int fd);

#endif  // _TASK_H_
