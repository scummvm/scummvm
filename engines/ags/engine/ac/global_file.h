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

#ifndef AGS_ENGINE_AC_GLOBALFILE_H
#define AGS_ENGINE_AC_GLOBALFILE_H

#include "ags/shared/util/file.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

int32_t FileOpen(const char *fnmm, Shared::FileOpenMode open_mode, Shared::FileWorkMode work_mode);
// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
int32_t FileOpenCMode(const char *fnmm, const char *cmode);
void  FileClose(int32_t handle);
void  FileWrite(int32_t handle, const char *towrite);
void  FileWriteRawLine(int32_t handle, const char *towrite);
void  FileRead(int32_t handle, char *toread);
int   FileIsEOF(int32_t handle);
int   FileIsError(int32_t handle);
void  FileWriteInt(int32_t handle, int into);
int   FileReadInt(int32_t handle);
int8  FileReadRawChar(int32_t handle);
int   FileReadRawInt(int32_t handle);
void  FileWriteRawChar(int32_t handle, int chartoWrite);

} // namespace AGS3

#endif
