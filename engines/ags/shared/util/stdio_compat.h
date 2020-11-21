/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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
