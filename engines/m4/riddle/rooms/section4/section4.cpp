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

#include "m4/riddle/rooms/section4/section4.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

Section4::Section4() : Rooms::Section() {
	add(401, &_room401);
	add(402, &_room402);
	add(403, &_room403);
	add(404, &_room404);
	add(405, &_room405);
	add(406, &_room406);
	add(407, &_room407);
	add(408, &_room408);
	add(409, &_room409);
	add(410, &_room410);
	add(413, &_room413);
	add(456, &_room456);
	add(493, &_room493);
	add(494, &_room494);
	add(495, &_room495);
}

void Section4::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
