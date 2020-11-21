//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AGS_CN_UTIL__STDIOCOMPAT_H
#define __AGS_CN_UTIL__STDIOCOMPAT_H

#include <stdio.h>
#include <stdint.h>

typedef int64_t file_off_t;

#ifdef __cplusplus
extern "C" {
#endif

int	 ags_fseek(FILE * stream, file_off_t offset, int whence);
file_off_t	 ags_ftell(FILE * stream);

int ags_file_exists(const char *path);
int ags_directory_exists(const char *path);
int ags_path_exists(const char *path);
file_off_t ags_file_size(const char *path);

#ifdef __cplusplus
}
#endif

#endif // __AGS_CN_UTIL__STDIOCOMPAT_H
