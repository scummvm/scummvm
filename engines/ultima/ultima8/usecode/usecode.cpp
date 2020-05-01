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

#include "ultima/ultima8/usecode/usecode.h"
#include "ultima/ultima8/kernel/core_app.h"

namespace Ultima {
namespace Ultima8 {

uint32 Usecode::get_class_event(uint32 classid, uint32 eventid) {
	if (get_class_size(classid) == 0) return 0;

	if (eventid >= get_class_event_count(classid)) {
		perr << "eventid too high: " << eventid << " >= " << get_class_event_count(classid) << " for class " << classid << Std::endl;
		CANT_HAPPEN();
	}

	const uint8 *data = get_class(classid);

	uint32 offset = 0;
	if (GAME_IS_U8) {
		offset = data[12 + (eventid * 4) + 0];
		offset += data[12 + (eventid * 4) + 1] << 8;
		offset += data[12 + (eventid * 4) + 2] << 16;
		offset += data[12 + (eventid * 4) + 3] << 24;
	} else if (GAME_IS_CRUSADER) {
		offset = data[20 + (eventid * 6) + 2];
		offset += data[20 + (eventid * 6) + 3] << 8;
		offset += data[20 + (eventid * 6) + 4] << 16;
		offset += data[20 + (eventid * 6) + 5] << 24;
	} else {
		CANT_HAPPEN_MSG("Invalid game type.");
	}

	return offset;
}

} // End of namespace Ultima8
} // End of namespace Ultima
