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

#include "m4/burger/rooms/section5/room511.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room511::SERIES1[] = {
	{ 0, "511b001a", 2, 255, -1, 0, &_state1,  1 },
	{ 0, "511b001b", 2, 255, -1, 0, &_state1,  2 },
	{ 0, "511b001c", 2, 255, -1, 0, &_state1,  3 },
	{ 0, "511b001d", 2, 255, -1, 0, &_state1,  4 },
	{ 0, "511b001e", 2, 255, -1, 0, &_state1,  5 },
	{ 0, "511b001f", 2, 255, -1, 0, &_state1,  6 },
	{ 0, "511b001g", 2, 255, -1, 0, &_state1,  7 },
	{ 0, "511b001h", 2, 255, -1, 0, &_state1,  8 },
	{ 0, "511b001e", 2, 255, -1, 0, &_state1,  9 },
	{ 0, "511b001j", 2, 255, -1, 0, &_state1, 10 },
	STREAM_BREAK_END
};

int32 Room511::_state1;


Room511::Room511() : Section5Room() {
	_state1 = 0;
}

void Room511::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room511::init() {
	_G(flags)[V194] = 1;
	kernel_trigger_dispatch_now(1);
}

void Room511::daemon() {
	if (_G(kernel).trigger == 1) {
		_state1 = imath_ranged_rand(1, 10);
		_val1 = imath_ranged_rand(1, 5);

		switch (_G(flags)[V206]) {
		case 5000:
			series_stream_with_breaks(SERIES1, "511bk03", 8, 1, 5016);
			break;

		case 5001:
			series_stream_with_breaks(SERIES1, "511bk07", 8, 1, 5016);
			break;

		case 5002:
			series_stream_with_breaks(SERIES1, "511bk04", 8, 1, 5016);
			break;

		case 5003:
			series_stream_with_breaks(SERIES1, "511bk08", 8, 1, 5016);
			break;

		case 5004:
			series_stream_with_breaks(SERIES1, "511bk06", 8, 1, 5016);
			break;

		case 5005:
			series_stream_with_breaks(SERIES1, "511bk05", 8, 1, 5016);
			break;

		default:
			series_stream_with_breaks(SERIES1, "511bk02", 8, 1, 5016);
			break;
		}
	} else {
		_G(kernel).continue_handling_trigger = true;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
