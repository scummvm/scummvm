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
#include "m4/burger/vars.h"
#include "m4/wscript/wst_regs.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section6Room::Section6Room() : Room() {
	Common::strcpy_s(_gerbilsName, "GERBILS");
	Common::strcpy_s(_gerbilsVerb, "LOOK AT");

	_gerbilHotspot.clear();
	_gerbilHotspot.vocab = _gerbilsName;
	_gerbilHotspot.verb = _gerbilsVerb;
	_gerbilHotspot.feet_x = 0x7fff;
	_gerbilHotspot.feet_y = 0x7fff;
	_gerbilHotspot.cursor_number = kArrowCursor;
}

HotSpotRec *Section6Room::custom_hotspot_which(int32 x, int32 y) {
	HotSpotRec *hotspot = Rooms::Room::custom_hotspot_which(x, y);
	if (hotspot)
		return &_wilburHotspot;

	if (_G(flags)[kHampsterState] != 6006 || !_gerbilTable ||
			!verifyMachineExists(_sectionMachine1) ||
			(_G(game).room_id != 602 && _G(game).room_id != 603 &&
				_G(game).room_id != 604))
		return nullptr;

	int x1, x2, x3, y1, y2, y3;
	int total1, total2, total3;
	int diff;

	if (_G(flags)[V266]) {
		x1 = 189;
		y1 = 232;
		x2 = 318;
		y2 = 219;
		x3 = 439;
		y3 = 232;
	} else {
		int index = _sectionMachine1->myAnim8->myRegs[IDX_CELS_INDEX];
		const GerbilPoint *points = _gerbilTable + index;

		diff = points[3]._x - points[0]._x;
		x1 = points[0]._x + (index % 3) * diff / 3;

		diff = points[4]._x - points[1]._x;
		x2 = points[1]._x + (index % 3) * diff / 3;

		diff = points[5]._x - points[2]._x;
		x3 = points[2]._x + (index % 3) * diff / 3;

		diff = points[3]._y - points[0]._y;
		y1 = points[0]._y + (index % 3) * diff / 3;

		diff = points[4]._y - points[1]._y;
		y2 = points[1]._y + (index % 3) * diff / 3;

		diff = points[5]._y - points[2]._y;
		y3 = points[2]._y + (index % 3) * diff / 3;
	}

	total1 = (x - x1) * (x - x1) + (y - y1) * (y - y1);
	total2 = (x - x2) * (x - x2) + (y - y2) * (y - y2);
	total3 = (x - x3) * (x - x3) + (y - y3) * (y - y3);

	return (total1 < 1600) || (total2 < 1600) < (total3 < 1600) ?
		&_gerbilHotspot : nullptr;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
