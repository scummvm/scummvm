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

#ifndef AGS_ENGINE_DYNOBJ__SCRIPTFILE_H
#define AGS_ENGINE_DYNOBJ__SCRIPTFILE_H

#include "ags/engine/ac/dynobj/cc_dynamic_object.h"
#include "ags/shared/util/file.h"

namespace AGS3 {

using namespace AGS; // FIXME later

#define scFileRead   1
#define scFileWrite  2
#define scFileAppend 3

struct sc_File final : ICCDynamicObject {
	int32_t             handle;

	static const Shared::FileOpenMode fopenModes[];
	static const Shared::FileWorkMode fworkModes[];

	int Dispose(const char *address, bool force) override;

	const char *GetType() override;

	int Serialize(const char *address, char *buffer, int bufsize) override;

	int OpenFile(const char *filename, int mode);
	void Close();

	sc_File();

	// Legacy support for reading and writing object values by their relative offset
	const char *GetFieldPtr(const char *address, intptr_t offset) override;
	void    Read(const char *address, intptr_t offset, void *dest, int size) override;
	uint8_t ReadInt8(const char *address, intptr_t offset) override;
	int16_t ReadInt16(const char *address, intptr_t offset) override;
	int32_t ReadInt32(const char *address, intptr_t offset) override;
	float   ReadFloat(const char *address, intptr_t offset) override;
	void    Write(const char *address, intptr_t offset, void *src, int size) override;
	void    WriteInt8(const char *address, intptr_t offset, uint8_t val) override;
	void    WriteInt16(const char *address, intptr_t offset, int16_t val) override;
	void    WriteInt32(const char *address, intptr_t offset, int32_t val) override;
	void    WriteFloat(const char *address, intptr_t offset, float val) override;
};

} // namespace AGS3

#endif
