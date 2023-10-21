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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/burger/rooms/section6/room606.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room606::SERIES1[] = {
	{  0, "606x001",  2, 100, -1, 0, nullptr, 0 },
	{ 15, "606w003a", 1, 255, -1, 0, &_state1,  0 },
	{ 15, "606w003b", 1, 255, -1, 0, &_state1,  1 },
	{ 15, "606w003c", 1, 255, -1, 0, &_state1,  2 },
	{ 15, "606w003d", 1, 255, -1, 0, &_state1,  3 },
	STREAM_BREAK_END
};

int32 Room606::_state1;

Room606::Room606() : Section6Room() {
	_state1 = 0;
}

void Room606::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room606::init() {
	kernel_trigger_dispatch_now(1);
}

void Room606::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_preload("606x001", 606);
		digi_preload("606w003a", 606);
		digi_preload("606w003b", 606);
		digi_preload("606w003c", 606);
		digi_preload("606w003d", 606);
		_state1 = imath_ranged_rand(0, 3);
		series_stream_with_breaks(SERIES1, "606laser", 8, 1, 6007);
		break;

	case 6007:
		_G(game).new_room = 608;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
