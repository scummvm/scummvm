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

#include "ags/engine/ac/dynobj/script_file.h"
#include "ags/engine/ac/global_file.h"

namespace AGS3 {

// CHECKME: actually NULLs here will be equal to kFile_Open & kFile_Read
const Shared::FileOpenMode sc_File::fopenModes[] =
{ Shared::kFile_Open/*CHECKME, was undefined*/, Shared::kFile_Open, Shared::kFile_CreateAlways, Shared::kFile_Create };
const Shared::FileWorkMode sc_File::fworkModes[] =
{ Shared::kFile_Read/*CHECKME, was undefined*/, Shared::kFile_Read, Shared::kFile_Write, Shared::kFile_Write };

int sc_File::Dispose(const char *address, bool force) {
	Close();
	delete this;
	return 1;
}

const char *sc_File::GetType() {
	return "File";
}

int sc_File::Serialize(const char *address, char *buffer, int bufsize) {
	// we cannot serialize an open file, so it will get closed
	return 0;
}

int sc_File::OpenFile(const char *filename, int mode) {
	handle = FileOpen(filename, fopenModes[mode], fworkModes[mode]);
	if (handle <= 0)
		return 0;
	return 1;
}

void sc_File::Close() {
	if (handle > 0) {
		FileClose(handle);
		handle = 0;
	}
}

sc_File::sc_File() {
	handle = 0;
}


const char *sc_File::GetFieldPtr(const char *address, intptr_t offset) {
	return address;
}

void sc_File::Read(const char *address, intptr_t offset, void *dest, int size) {
}

uint8_t sc_File::ReadInt8(const char *address, intptr_t offset) {
	return 0;
}

int16_t sc_File::ReadInt16(const char *address, intptr_t offset) {
	return 0;
}

int32_t sc_File::ReadInt32(const char *address, intptr_t offset) {
	return 0;
}

float sc_File::ReadFloat(const char *address, intptr_t offset) {
	return 0.0;
}

void sc_File::Write(const char *address, intptr_t offset, void *src, int size) {
}

void sc_File::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
}

void sc_File::WriteInt16(const char *address, intptr_t offset, int16_t val) {
}

void sc_File::WriteInt32(const char *address, intptr_t offset, int32_t val) {
}

void sc_File::WriteFloat(const char *address, intptr_t offset, float val) {
}

} // namespace AGS3
