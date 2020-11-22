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

//=============================================================================
//
// Platform-independent Path functions
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_PATH_H
#define AGS_SHARED_UTIL_PATH_H

#include "util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

namespace Path {
// Tells if the given path is a directory
bool    IsDirectory(const String &directory);
// Tells if the given path is a file
bool    IsFile(const String &filename);
// Tells if the given path is file or directory;
// may be used to check if it's valid to use
bool    IsFileOrDir(const String &filename);

// Makes a platform-dependant path comparison.
// This takes into consideration platform's filename case (in)sensivity and
// DOS-compatible 8.3 filenames;
// The result value corresponds to stdlib strcmp function.
int     ComparePaths(const String &path1, const String &path2);

// Returns path to the actual directory, referenced by given path;
// if path is a directory, returns path unchanged, if path is a file, returns
// parent directory containing that file.
String  GetDirectoryPath(const String &path);
// Tells if the path points to the parent path's location or lower directory;
// return FALSE if the path points to outside of the parent location.
bool    IsSameOrSubDir(const String &parent, const String &path);

// Makes a path have only '/' slashes; this is to make it easier to work
// with path, knowing it contains only one type of directory separators
void    FixupPath(String &path);
// Fixups path and removes trailing slash
String  MakePathNoSlash(const String &path);
// Fixups path and adds trailing slash if it's missing
String  MakeTrailingSlash(const String &path);
// Converts any path to an absolute path; relative paths are assumed to
// refer to the current working directory.
String  MakeAbsolutePath(const String &path);
// Tries to create a relative path that would point to 'path' location
// if walking out of the 'base'. Returns empty string on failure.
String  MakeRelativePath(const String &base, const String &path);
// Concatenates parent and relative paths
String  ConcatPaths(const String &parent, const String &child);

// Subsitutes illegal characters with '_'. This function uses a combined set
// of illegal chars from all the supported platforms to make a name that
// could be copied across systems without problems.
String  FixupSharedFilename(const String &filename);

// Converts filepath into ASCII variant; returns empty string on failure
String  GetPathInASCII(const String &path);
// Converts filepath from command line's argument into ASCII variant
String  GetCmdLinePathInASCII(const char *arg, int arg_index);
} // namespace Path

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
