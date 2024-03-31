// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _WAIT_H_
#define _WAIT_H_

#include <sys/signal.h>
#include <sys/types.h>

// Do not hang if no status is available, return immediately.
#define WNOHANG 1
// Report status of stopped child process.
#define WUNTRACED 2

// True if the child exited normally.
#define WIFEXITED(w) (((w)&0xff) == 0)
// True if the child exited due to uncaught signal.
#define WIFSIGNALED(w) (((w)&0x7f) > 0 && (((w)&0x7f) < 0x7f))
// True if the child is currently stopped.
#define WIFSTOPPED(w) (((w)&0xff) == 0x7f)
// Returns the exit status.
#define WEXITSTATUS(w) (((w) >> 8) & 0xff)
// Returns the signal numver that caused the process to terminate.
#define WTERMSIG(w) ((w)&0x7f)
// Returns the signal number that caused the process to stop.
#define WSTOPSIG WEXITSTATUS

pid_t wait(int *wstatus);

pid_t waitpid(pid_t pid, int *wstatus, int options);

typedef enum {
    P_ALL,
    P_PGID,
    P_PID,
} idtype_t;

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

#endif  // _WAIT_H_
