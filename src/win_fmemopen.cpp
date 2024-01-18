/*
 * Copyright (c) 2017  Joachim Nilsson <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef MINGW
#include <cstdio>
#include <fcntl.h>
#include <windows.h>

FILE *fmemopen(void *buf, size_t len, const char *type) {
    int fd;
    FILE *fp;
    char tp[MAX_PATH - 13];
    char fn[MAX_PATH + 1];
    HANDLE h;

    if (!GetTempPath(sizeof(tp), tp))
        return NULL;

    if (!GetTempFileName(tp, "godot-csound", 0, fn))
        return NULL;

    h = CreateFile(fn, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (INVALID_HANDLE_VALUE == h)
        return NULL;

    fd = _open_osfhandle((intptr_t)h, _O_APPEND);
    if (fd < 0) {
        CloseHandle(h);
        return NULL;
    }

    fp = fdopen(fd, "w+");
    if (!fp) {
        CloseHandle(h);
        return NULL;
    }

    fwrite(buf, len, 1, fp);
    rewind(fp);

    return fp;
}
#endif
