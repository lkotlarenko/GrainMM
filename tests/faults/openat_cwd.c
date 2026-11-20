/* SPDX-License-Identifier: GPL-3.0-only */
/* Original fault fixture: emulate a wrapper that resolves openat paths against CWD. */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>

int openat(int directory, const char *path, int flags, ...) {
    (void)directory;
    if (flags & O_CREAT) {
        va_list arguments;
        va_start(arguments, flags);
        const mode_t mode = va_arg(arguments, mode_t);
        va_end(arguments);
        return open(path, flags, mode);
    }
    return open(path, flags);
}
