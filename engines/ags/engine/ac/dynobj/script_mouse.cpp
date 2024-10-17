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
#include "ags/engine/ac/dynobj/script_mouse.h"
#include "ags/shared/script/cc_common.h" // cc_error

namespace AGS3 {

int32_t ScriptMouse::ReadInt32(void *address, intptr_t offset) {
	switch (offset) {
	case 0:
		return x;
	case 4:
		return y;
	default:
		cc_error("ScriptMouse: unsupported variable offset %d", offset);
		return 0;
	}
}

void ScriptMouse::WriteInt32(void *address, intptr_t offset, int32_t val) {
	switch (offset) {
	case 0:
	case 4:
		debug_script_warn("ScriptMouse: attempt to write in readonly variable at offset %d, value", offset, val);
		break;
	default:
		cc_error("ScriptMouse: unsupported variable offset %d", offset);
		break;
	}
}

} // namespace AGS3
