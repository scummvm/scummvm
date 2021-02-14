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

#ifndef _AGS_ENGINE_AC_STATICOBJ_AGSSTATICOBJECT_H
#define _AGS_ENGINE_AC_STATICOBJ_AGSSTATICOBJECT_H

#include "ags/engine/ac/statobj/staticobject.h"

namespace AGS3 {

struct AGSStaticObject : public ICCStaticObject {
	~AGSStaticObject() override {}

	// Legacy support for reading and writing object values by their relative offset
	const char *GetFieldPtr(const char *address, intptr_t offset) override;
	void    Read(const char *address, intptr_t offset, void *dest, int size) override;
	uint8 ReadInt8(const char *address, intptr_t offset) override;
	int16 ReadInt16(const char *address, intptr_t offset) override;
	int ReadInt32(const char *address, intptr_t offset) override;
	float   ReadFloat(const char *address, intptr_t offset) override;
	void    Write(const char *address, intptr_t offset, void *src, int size) override;
	void    WriteInt8(const char *address, intptr_t offset, uint8 val) override;
	void    WriteInt16(const char *address, intptr_t offset, int16 val) override;
	void    WriteInt32(const char *address, intptr_t offset, int val) override;
	void    WriteFloat(const char *address, intptr_t offset, float val) override;
};

// Wrapper around script's "Game" struct, managing access to its variables
struct StaticGame : public AGSStaticObject {
	void    WriteInt32(const char *address, intptr_t offset, int val) override;
};

extern AGSStaticObject GlobalStaticManager;
extern StaticGame      GameStaticManager;

} // namespace AGS3

#endif
