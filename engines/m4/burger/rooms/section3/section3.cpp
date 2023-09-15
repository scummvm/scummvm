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

#include "m4/burger/rooms/section3/section3.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section3::Section3() : Rooms::Section() {
	add(301, &_room301);
	add(302, &_room302);
	add(303, &_room303);
	add(304, &_room304);
	add(305, &_room305);
	add(306, &_room306);
	add(307, &_room307);
	add(310, &_room310);
	add(319, &_room319);
}

void Section3::daemon() {
	// TODO
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
