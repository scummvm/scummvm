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

#include "m4/burger/rooms/section5/section5_room.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"
#include "m4/wscript/wst_regs.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Section5Room::preload() {
	Rooms::Room::preload();
	_G(flags)[V186] = 0;
	_initFlag = false;
	Section5::_digiName = nullptr;
	Section5::_bgDigiName = nullptr;
}

void Section5Room::init() {
	if (_initFlag) {
		_initFlag = false;
		kernel_timing_trigger(120, 5004);
	}

	if (Section5::_digiName != 0) {
		digi_stop(3);
		digi_unload(Section5::_digiName);
	}
	Section5::_digiName = nullptr;

	int vol = 125;

	switch (_G(game).room_id) {
	case 502:
		if (_G(flags)[kFireplaceHasFire])
			Section5::_digiName = "500_002";
		break;

	case 505:
		Section5::_digiName = "505_002";
		break;

	case 506:
		Section5::_digiName = _G(flags)[V218] == 5003 ? "500_002" : "506_005";
		break;

	case 508:
		Section5::_digiName = "500_029";
		break;

	default:
		break;
	}

	if (Section5::_digiName) {
		digi_preload(Section5::_digiName);
		digi_play_loop(Section5::_digiName, 3, vol);
	}
}

HotSpotRec *Section5Room::custom_hotspot_which(int32 x, int32 y) {
	HotSpotRec *hotspot = Rooms::Room::custom_hotspot_which(x, y);
	if (hotspot != nullptr || _G(flags)[V186] == 0 || !_borkStairs ||
		(_G(game).room_id != 502 && _G(game).room_id != 505))
		return hotspot;

	assert(_borkTable);
	int index = _borkStairs.regs()[IDX_CELS_INDEX];
	int x1 = _borkTable[index]._x;
	int y1 = _borkTable[index]._y;
	int total = (x - x1) * (x - x1) + (y - y1) * (y - y1);

	return (total < _borkThreshold) ? &_borkHotspot : nullptr;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
