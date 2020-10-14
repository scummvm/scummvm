/*
 * Copyright (c) 2009 Keith Bauer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "fcaseopen.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

#ifndef WIN32

// r must have strlen(path) + 2 bytes
static int casepath(char const *path, char *r)
{
    size_t l = strlen(path);
    char *p = alloca(l + 1);
    strcpy(p, path);
    size_t rl = 0;

    DIR *d;
    if (p[0] == '/')
    {
        d = opendir("/");
        p = p + 1;
    }
    else
    {
        d = opendir(".");
        r[0] = '.';
        r[1] = 0;
        rl = 1;
    }

    int last = 0;
    char *c = strsep(&p, "/");
    while (c)
    {
        if (!d)
        {
            return 0;
        }

        if (last)
        {
            closedir(d);
            return 0;
        }

        r[rl] = '/';
        rl += 1;
        r[rl] = 0;

        struct dirent *e = readdir(d);
        while (e)
        {
            if (strcasecmp(c, e->d_name) == 0)
            {
                strcpy(r + rl, e->d_name);
                rl += strlen(e->d_name);

                closedir(d);
                d = opendir(r);

                break;
            }

            e = readdir(d);
        }

        if (!e)
        {
            strcpy(r + rl, c);
            rl += strlen(c);
            last = 1;
        }

        c = strsep(&p, "/");
    }

    if (d) closedir(d);
    return 1;
}
#endif

FILE *fcaseopen(char const *path, char const *mode)
{
    FILE *f = fopen(path, mode);
#ifndef WIN32
    if (!f)
    {
        char *r = alloca(strlen(path) + 2);
        if (casepath(path, r))
        {
            f = fopen(r, mode);
        }
    }
#endif
    return f;
}

void casechdir(char const *path)
{
#ifndef WIN32
    char *r = alloca(strlen(path) + 2);
    if (casepath(path, r))
    {
        chdir(r);
    }
    else
    {
        errno = ENOENT;
    }
#else
    _chdir(path);
#endif
}
