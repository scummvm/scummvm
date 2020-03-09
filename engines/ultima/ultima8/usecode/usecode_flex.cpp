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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/usecode/usecode_flex.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/games/game_info.h"

namespace Ultima {
namespace Ultima8 {

const uint8 *UsecodeFlex::get_class(uint32 classid) {
	const uint8 *obj = get_object_nodel(classid + 2);
	return obj;
}

uint32 UsecodeFlex::get_class_size(uint32 classid) {
	uint32 size = get_size(classid + 2);
	return size;
}

const char *UsecodeFlex::get_class_name(uint32 classid) {
	if (get_size(classid + 2) > 0) {
		const uint8 *name_object = get_object_nodel(1);
		return reinterpret_cast<const char *>(name_object + 4 + (13 * classid));
	} else {
		return nullptr;
	}
}

uint32 UsecodeFlex::get_class_base_offset(uint32 classid) {
	if (get_size(classid + 2) == 0) return 0;

	if (GAME_IS_U8) {
		return 0x0C;
	} else if (GAME_IS_CRUSADER) {
		const uint8 *obj = get_object_nodel(classid + 2);
		uint32 offset = obj[8];
		offset += obj[9] << 8;
		offset += obj[10] << 16;
		offset += obj[11] << 24;
		offset--;
		return offset;
	} else {
		CANT_HAPPEN_MSG("Invalid game type.");
		return 0;
	}
}

uint32 UsecodeFlex::get_class_event_count(uint32 classid) {
	if (get_size(classid + 2) == 0) return 0;

	if (GAME_IS_U8) {
		return 32;
	} else if (GAME_IS_CRUSADER) {
		return (get_class_base_offset(classid) + 19) / 6;
	} else {
		CANT_HAPPEN_MSG("Invalid game type.");
		return 0;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
