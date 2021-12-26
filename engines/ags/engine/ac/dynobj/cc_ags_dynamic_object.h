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

#ifndef AGS_ENGINE_AC_DYNOBJ_CCDYNAMIC_OBJECT_H
#define AGS_ENGINE_AC_DYNOBJ_CCDYNAMIC_OBJECT_H

#include "ags/engine/ac/dynobj/cc_dynamic_object.h"

namespace AGS3 {

struct AGSCCDynamicObject : ICCDynamicObject {
protected:
	virtual ~AGSCCDynamicObject() {}
public:
	// default implementation
	int Dispose(const char *address, bool force) override;

	// TODO: pass savegame format version
	virtual void Unserialize(int index, const char *serializedData, int dataSize) = 0;

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

protected:
	// Savegame serialization
	// TODO: reimplement with the proper memory stream?!
	int bytesSoFar = 0;
	int totalBytes = 0;
	char *serbuffer = nullptr;

	void StartSerialize(char *sbuffer);
	void SerializeInt(int val);
	void SerializeFloat(float val);
	int  EndSerialize();
	void StartUnserialize(const char *sbuffer, int pTotalBytes);
	int  UnserializeInt();
	float UnserializeFloat();

};

} // namespace AGS3

#endif
