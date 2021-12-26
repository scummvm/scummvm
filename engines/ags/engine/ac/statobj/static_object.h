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
// A stub class for "managing" static global objects exported to script.
// This may be temporary solution (oh no, not again :P) that could be
// replaced by the use of dynamic objects in the future.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_STATOBJ_STATICOBJECT_H
#define AGS_ENGINE_AC_STATOBJ_STATICOBJECT_H

#include "ags/shared/core/types.h"

namespace AGS3 {

struct ICCStaticObject {
	virtual ~ICCStaticObject() {}

	// Legacy support for reading and writing object values by their relative offset
	virtual const char *GetFieldPtr(const char *address, intptr_t offset) = 0;
	virtual void    Read(const char *address, intptr_t offset, void *dest, int size) = 0;
	virtual uint8_t ReadInt8(const char *address, intptr_t offset) = 0;
	virtual int16_t ReadInt16(const char *address, intptr_t offset) = 0;
	virtual int32_t ReadInt32(const char *address, intptr_t offset) = 0;
	virtual float   ReadFloat(const char *address, intptr_t offset) = 0;
	virtual void    Write(const char *address, intptr_t offset, void *src, int size) = 0;
	virtual void    WriteInt8(const char *address, intptr_t offset, uint8_t val) = 0;
	virtual void    WriteInt16(const char *address, intptr_t offset, int16_t val) = 0;
	virtual void    WriteInt32(const char *address, intptr_t offset, int32_t val) = 0;
	virtual void    WriteFloat(const char *address, intptr_t offset, float val) = 0;
};

} // namespace AGS3

#endif
