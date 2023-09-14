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

#include "m4/burger/rooms/section2/section2.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section2::Section2() : Rooms::Section() {
	add(204, &_room204);
	add(207, &_room207);
}

void Section2::daemon() {
	switch (_G(kernel).trigger) {
	case 2001:
		_G(game).new_room = 204;
		break;

	case 2002:
		_G(game).new_room = 207;
		break;

	default:
		break;
	}

	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
