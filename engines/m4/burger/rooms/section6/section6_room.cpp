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

#include "m4/burger/rooms/section6/section6_room.h"
#include "m4/gui/gui_mouse.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section6Room::Section6Room() : Room() {
	Common::strcpy_s(_gerbilsName, "GERBILS");
	Common::strcpy_s(_gerbilsVerb, "LOOK AT");

	_gerbilsHotspot.clear();
	_gerbilsHotspot.vocab = _gerbilsName;
	_gerbilsHotspot.verb = _gerbilsVerb;
	_gerbilsHotspot.feet_x = 0x7fff;
	_gerbilsHotspot.feet_y = 0x7fff;
	_gerbilsHotspot.cursor_number = kArrowCursor;
}

HotSpotRec *Section6Room::custom_hotspot_which(int x, int y) {
	HotSpotRec *hotspot = walker_spotter(x, y);
	if (hotspot)
		return hotspot;

	return nullptr;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
