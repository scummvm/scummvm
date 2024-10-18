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
// Wrapper around script "Mouse" struct, managing access to its variables.
//
//=============================================================================
#ifndef AGS_ENGINE_DYNOBJ__SCRIPTMOUSE_H
#define AGS_ENGINE_DYNOBJ__SCRIPTMOUSE_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {

struct ScriptMouse : public AGSCCStaticObject {
	int x;
	int y;

	int32_t ReadInt32(void *address, intptr_t offset) override;
	void WriteInt32(void *address, intptr_t offset, int32_t val) override;
};

} // namespace AGS3

#endif
