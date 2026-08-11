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

#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/shared/script/cc_common.h" // cc_error

namespace AGS3 {

int32_t ScriptSystem::ReadInt32(void *address, intptr_t offset) {
	const int index = offset / sizeof(int32_t);
	switch (index) {
	case 0:
		return width;
	case 1:
		return height;
	case 2:
		return coldepth;
	case 3:
		return os;
	case 4:
		return windowed;
	case 5:
		return vsync;
	case 6:
		return viewport_width;
	case 7:
		return viewport_height;
	default:
		cc_error("ScriptSystem: unsupported variable offset %d", offset);
		return 0;
	}
}

void ScriptSystem::WriteInt32(void *address, intptr_t offset, int32_t val) {
	const int index = offset / sizeof(int32_t);
	switch (index) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
		debug_script_warn("ScriptSystem: attempt to write in readonly variable at offset %d, value %d", offset, val);
		break;
	case 5:
		vsync = val;
		break;
	default:
		cc_error("ScriptSystem: unsupported variable offset %d", offset);
		break;
	}
}

} // namespace AGS3
