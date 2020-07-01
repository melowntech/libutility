/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "../unistd_compat.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

// windows's ReadFile/WriteFile (with overlapped), unline linux pread/pwrite,
// modifies the current read position in the file
// but are otherwise threadsafe
// use with care

int pread(int fd, void* buf, size_t count, off_t offset)
{
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.OffsetHigh =
        (DWORD)((offset & 0xFFFFFFFF00000000LL) >> 32);
    overlapped.Offset =
        (DWORD)(offset & 0xFFFFFFFFLL);

    HANDLE file = (HANDLE)_get_osfhandle(fd);
    if (file == INVALID_HANDLE_VALUE)
        return -1;

    DWORD bytes = 0;
    BOOL res = ReadFile(file, buf, count, &bytes, &overlapped);

    if (res)
        return bytes;

    DWORD err = GetLastError();
    if (err == ERROR_HANDLE_EOF)
        return bytes; // ERROR_HANDLE_EOF is still success

    return -1;
}

int pwrite(int fd, const void* buf, size_t count, off_t offset)
{
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.OffsetHigh =
        (DWORD)((offset & 0xFFFFFFFF00000000LL) >> 32);
    overlapped.Offset =
        (DWORD)(offset & 0xFFFFFFFFLL);

    HANDLE file = (HANDLE)_get_osfhandle(fd);
    if (file == INVALID_HANDLE_VALUE)
        return -1;

    DWORD bytes = 0;
    BOOL res = WriteFile(file, buf, count, &bytes, &overlapped);

    if (res)
        return bytes;

    DWORD err = GetLastError();
    if (err == ERROR_HANDLE_EOF)
        return bytes; // ERROR_HANDLE_EOF is still success

    return -1;
}

int ftruncate(int fd, off_t length)
{
    HANDLE file = (HANDLE)_get_osfhandle(fd);
    if (file == INVALID_HANDLE_VALUE)
        return -1;
    LARGE_INTEGER pos;
    pos.QuadPart = length;
    LARGE_INTEGER res;
    res.QuadPart = 0;
    if (!SetFilePointerEx(file, pos, &res, FILE_BEGIN))
        return -1;
    if (!SetEndOfFile(file))
        return -1;
    return 0;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    // this is not threadsafe:
    // the localtime function writes into global variable
    // that is _later on_ copied into our buffer
    // if you are serious about this, use linux instead
    auto p = ::localtime(timep);
    if (p)
        *result = *p;
    return p ? result : nullptr;
}

#ifdef __cplusplus
}
#endif

