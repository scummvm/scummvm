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

// Wrapper around script "System" struct, managing access to its variables.
// ScriptSystem is a readable/writeable struct which had been exposed to
// script in older versions of API (deprecated).
// WARNING: it *MUST* keep its size exact to avoid breaking address offsets
// when running old scripts. In case of emergency you may use its reserved
// fields, but it's not recommended to do, as this struct is not a part of
// the modern API anymore.

#ifndef AGS_ENGINE_DYNOBJ_SCRIPT_SYSTEM_H
#define AGS_ENGINE_DYNOBJ_SCRIPT_SYSTEM_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {

struct ScriptSystem : AGSCCStaticObject {
	int width = 0; // game screen width
	int height = 0; // game screen height
	int coldepth = 0; // game's color depth, in bits per pixel (8, 16, 32)
	int os = 0; // operating system's code (see eScriptSystemOSID)
	int windowed = 0; // windowed/fullscreen flag
	int vsync = 0; // vertical sync flag
	int viewport_width = 0; // game viewport width (normal or letterboxed)
	int viewport_height = 0; // game viewport height (normal or letterboxed)
	char aci_version[10]{}; // engine version string (informational)
	int reserved[5]{}; // reserved fields

	int32_t ReadInt32(void *address, intptr_t offset) override;
	void	WriteInt32(void *address, intptr_t offset, int32_t val) override;
};

} // namespace AGS3

#endif
