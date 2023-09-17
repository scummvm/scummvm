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

#include "m4/burger/rooms/section3/section3_room.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Section3Room::preload() {
	Rooms::Room::preload();
	_digiName = nullptr;
}

void Section3Room::setupDigi() {
	if (_digiName) {
		digi_stop(3);
		digi_unload(_digiName);
	}

	_digiVolume = 75;
	_digiName = getDigi();

	if (_digiName) {
		digi_preload(_digiName);
		digi_play_loop(_digiName, 3, _digiVolume);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
