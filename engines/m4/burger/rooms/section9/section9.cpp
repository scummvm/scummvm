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

#include "m4/burger/rooms/section9/section9.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section9::Section9() : Rooms::Section() {
	add(901, &_room901);
	add(902, &_room902);
	add(903, &_room903);
	add(904, &_room904);
	add(951, &_room951);
	add(971, &_room971);
}

void Section9::daemon() {
	switch (_G(kernel).trigger) {
	case 9002:
		_G(game).new_room = 902;
		break;
	case 9004:
		_G(game).new_room = 904;
		break;
	case 9005:
		_G(game).new_room = 951;
		break;
	case 9006:
		_G(game).new_room = 971;
		break;
	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
