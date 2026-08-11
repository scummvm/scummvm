/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_SHARED_UTIL_STRING_COMPAT_H
#define AGS_SHARED_UTIL_STRING_COMPAT_H

#include "ags/shared/core/types.h"

namespace AGS3 {

char *ags_strlwr(char *s);
char *ags_strupr(char *s);
int ags_stricmp(const char *, const char *);
int ags_strnicmp(const char *, const char *, size_t);
char *ags_strdup(const char *s);
int ags_strncpy_s(char *dest, size_t dest_sz, const char *src, size_t count);

} // namespace AGS3

#endif
