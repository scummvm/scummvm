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

#ifndef AGS_SHARED_UTIL_STDIO_COMPAT_H
#define AGS_SHARED_UTIL_STDIO_COMPAT_H

#include "common/stream.h"
#include "common/fs.h"

namespace AGS3 {

typedef int64 file_off_t;

// Size of the buffer enough to accomodate a UTF-8 path
const size_t MAX_PATH_SZ = 1024;

extern Common::FSNode getFSNode(const char *path);

extern int  ags_fseek(Common::Stream *stream, file_off_t offset, int whence);
extern file_off_t ags_ftell(Common::Stream *stream);

extern int ags_file_exists(const char *path);
extern int ags_directory_exists(const char *path);
extern int ags_path_exists(const char *path);
extern file_off_t ags_file_size(const char *path);

} // namespace AGS3

#endif
