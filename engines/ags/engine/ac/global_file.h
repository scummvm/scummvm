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

#ifndef AGS_ENGINE_AC_GLOBAL_FILE_H
#define AGS_ENGINE_AC_GLOBAL_FILE_H

#include "ags/shared/util/file.h"

namespace AGS3 {
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

int FileOpen(const char *fnmm, Shared::FileOpenMode open_mode, Shared::FileWorkMode work_mode);
// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
int FileOpenCMode(const char *fnmm, const char *cmode);
void  FileClose(int handle);
void  FileWrite(int handle, const char *towrite);
void  FileWriteRawLine(int handle, const char *towrite);
void  FileRead(int handle, char *toread);
int   FileIsEOF(int handle);
int   FileIsError(int handle);
void  FileWriteInt(int handle, int into);
int   FileReadInt(int handle);
char  FileReadRawChar(int handle);
int   FileReadRawInt(int handle);
void  FileWriteRawChar(int handle, int chartoWrite);

} // namespace AGS3

#endif
