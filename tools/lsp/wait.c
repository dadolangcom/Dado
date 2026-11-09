#ifdef _WIN32
/* Windows (P18): stdin is the editor's pipe; a pipe is polled with
   PeekNamedPipe, anything else waited on as a handle. */
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <io.h>
#include "wait.h"

/* 1 when stdin has bytes (or its end), 0 when `ms` passed; ms < 0 waits. */
int32_t lsp_wait(int32_t ms) {
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (GetFileType(h) == FILE_TYPE_PIPE) {
        DWORD start = GetTickCount();
        for (;;) {
            DWORD avail = 0;
            if (!PeekNamedPipe(h, 0, 0, 0, &avail, 0)) return 1; /* closed: its end */
            if (avail > 0) return 1;
            if (ms >= 0 && GetTickCount() - start >= (DWORD)ms) return 0;
            Sleep(1);
        }
    }
    return WaitForSingleObject(h, ms < 0 ? INFINITE : (DWORD)ms) == WAIT_OBJECT_0;
}

/* Up to n bytes of stdin; 0 at its end. */
int64_t lsp_read(void *buf, int64_t n) {
    int r = _read(0, buf, (unsigned)(n > 0x40000000 ? 0x40000000 : n));
    return r < 0 ? 0 : (int64_t)r;
}
#else
#define _POSIX_C_SOURCE 200809L
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include "wait.h"

/* 1 when stdin has bytes (or its end), 0 when `ms` passed; ms < 0 waits. */
int32_t lsp_wait(int32_t ms) {
    struct pollfd p = { 0, POLLIN, 0 };
    for (;;) {
        int r = poll(&p, 1, ms);
        if (r < 0 && errno == EINTR) continue;
        return r != 0;
    }
}

/* Up to n bytes of stdin; 0 at its end. */
int64_t lsp_read(void *buf, int64_t n) {
    for (;;) {
        ssize_t r = read(0, buf, (size_t)n);
        if (r < 0 && errno == EINTR) continue;
        return r < 0 ? 0 : (int64_t)r;
    }
}
#endif
