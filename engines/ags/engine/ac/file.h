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

//=============================================================================
//
// Script File API implementation.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_FILE_H
#define AGS_ENGINE_AC_FILE_H

#include "common/std/memory.h"
#include "ags/engine/ac/dynobj/script_file.h"
#include "ags/engine/ac/runtime_defines.h"

namespace AGS3 {

using AGS::Shared::Stream;

int     File_Exists(const char *fnmm);
int     File_Delete(const char *fnmm);
void *sc_OpenFile(const char *fnmm, int mode);
const char *File_ResolvePath(const char *fnmm);
void    File_Close(sc_File *fil);
void    File_WriteString(sc_File *fil, const char *towrite);
void    File_WriteInt(sc_File *fil, int towrite);
void    File_WriteRawChar(sc_File *fil, int towrite);
void    File_WriteRawInt(sc_File *fil, int towrite);
void    File_WriteRawLine(sc_File *fil, const char *towrite);
void    File_ReadRawLine(sc_File *fil, char *buffer);
const char *File_ReadRawLineBack(sc_File *fil);
void    File_ReadString(sc_File *fil, char *toread);
const char *File_ReadStringBack(sc_File *fil);
int     File_ReadInt(sc_File *fil);
int     File_ReadRawChar(sc_File *fil);
int     File_ReadRawInt(sc_File *fil);
int     File_Seek(sc_File *fil, int offset, int origin);
int     File_GetEOF(sc_File *fil);
int     File_GetError(sc_File *fil);
int     File_GetPosition(sc_File *fil);

struct ScriptFileHandle {
	std::unique_ptr<Stream> stream;
	int32_t  handle = 0;
};

ScriptFileHandle *check_valid_file_handle_ptr(Stream *stream_ptr, const char *operation_name);
ScriptFileHandle *check_valid_file_handle_int32(int32_t handle, const char *operation_name);
Stream *get_valid_file_stream_from_handle(int32_t handle, const char *operation_name);

} // namespace AGS3

#endif
