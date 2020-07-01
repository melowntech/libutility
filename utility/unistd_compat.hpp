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
#ifndef utility_unistd_compat_included_dsfh5jk4jdfh
#define utility_unistd_compat_included_dsfh5jk4jdfh

#ifdef _WIN32

#include <Windows.h>
#include <io.h> // lseek
#include <sys/types.h> // off_t
#include <sys/stat.h> // S_IFREG
#include <time.h> // localtime

#ifndef S_IRGRP
#define S_IRGRP 040
#endif
#ifndef S_IWGRP
#define S_IWGRP 020
#endif
#ifndef S_IROTH
#define S_IROTH 04
#endif
#ifndef S_IWOTH
#define S_IWOTH 02
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int pread(int fd, void *buf, size_t count, off_t offset);
int pwrite(int fd, const void *buf, size_t count, off_t offset);
int ftruncate(int fd, off_t length);
struct tm *localtime_r(const time_t *timep, struct tm *result);

#ifdef __cplusplus
}
#endif

#else // _WIN32

#include <unistd.h>

#endif // _WIN32

#endif // utility_unistd_compat_included_dsfh5jk4jdfh
