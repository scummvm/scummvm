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

#include "m4/burger/rooms/section6/room605.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak SERIES1[] = {
	{  0, "605x001", 1, 255, -1, 0, nullptr, 0 },
	{  1, "605_002", 3, 155, -1, 0, nullptr, 0 },
	{ 31, "605_001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{  0, "605x001", 1, 255, -1, 0, nullptr, 0 },
	{  1, "605_002", 3, 155, -1, 0, nullptr, 0 },
	{ 31, "605_001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room605::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room605::init() {
	kernel_trigger_dispatch_now(1);
}

void Room605::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		if (player_been_here(605)) {
			digi_preload_stream_breaks(SERIES2);
			_val1 = imath_ranged_rand(0, 4);
			series_stream_with_breaks(SERIES2, "605radar", 6, 1, 6007);
		} else {
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "605radar", 6, 1, 6007);
		}
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
