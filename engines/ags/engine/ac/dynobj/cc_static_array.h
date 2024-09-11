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
// CCStaticArray manages access to an array of script objects,
// where an element's size counted by script's bytecode may differ from the
// real element size in the engine's memory.
// The purpose of this is to remove size restriction from the engine's structs
// exposed to scripts.
// NOTE: on the other hand, similar effect could be achieved by separating
// object data into two or more structs, where "base" structs are stored in
// the exposed arrays (part of API), while extending structs are stored
// separately. This is more an issue of engine data design.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_STATIC_ARRAY_H
#define AGS_ENGINE_AC_DYNOBJ_STATIC_ARRAY_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {


struct CCStaticArray : public AGSCCStaticObject {
public:
	~CCStaticArray() override {}

	void Create(IScriptObject *mgr, size_t elem_script_size, size_t elem_mem_size, size_t elem_count = SIZE_MAX /*unknown*/);

	inline IScriptObject *GetObjectManager() const {
		return _mgr;
	}

	// Legacy support for reading and writing object values by their relative offset
	inline void *GetElementPtr(void *address, intptr_t legacy_offset) {
		return static_cast<uint8_t *>(address) + (legacy_offset / _elemScriptSize) * _elemMemSize;
	}

	void	*GetFieldPtr(void *address, intptr_t offset) override;
	void 	Read(void *address, intptr_t offset, uint8_t *dest, size_t size) override;
	uint8_t ReadInt8(void *address, intptr_t offset) override;
	int16_t ReadInt16(void *address, intptr_t offset) override;
	int32_t ReadInt32(void *address, intptr_t offset) override;
	float 	ReadFloat(void *address, intptr_t offset) override;
	void 	Write(void *address, intptr_t offset, const uint8_t *src, size_t size) override;
	void 	WriteInt8(void *address, intptr_t offset, uint8_t val) override;
	void 	WriteInt16(void *address, intptr_t offset, int16_t val) override;
	void 	WriteInt32(void *address, intptr_t offset, int32_t val) override;
	void 	WriteFloat(void *address, intptr_t offset, float val) override;

private:
	IScriptObject	 *_mgr = nullptr;
	size_t			 _elemScriptSize = 0u;
	size_t			 _elemMemSize = 0u;
	size_t			 _elemCount = 0u;
};

} // namespace AGS3

#endif
