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

#include "m4/burger/rooms/section3/room306.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room306::SERIES1[] = {
	{  0, "608_003",  3, 100, -1, 0, nullptr, 0 },
	{  2, "306f003a", 1, 255, -1, 0, nullptr, 0 },
	{ 33, "608_001",  2, 255, -1, 0, nullptr, 0 },
	{ 48, "608_002",  2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room306::SERIES2[] = {
	{  0, "608_003",  3, 100, -1, 0, nullptr, 0 },
	{  2, "306f001a", 1, 255, -1, 0, nullptr, 0 },
	{ 33, "608_001",  2, 255, -1, 0, nullptr, 0 },
	{ 48, "608_002",  2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room306::SERIES3[] = {
	{  0, "608_003",  3, 100, -1, 0, nullptr, 0 },
	{  1, "306f002a", 1, 255, -1, 0, nullptr, 0 }, 
	{ 33, "608_001",  2, 255, -1, 0, nullptr, 0 }, 
	{ 48, "608_002",  2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room306::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room306::init() {
	pal_cycle_init(101, 110, 6);
	kernel_trigger_dispatch_now(3007);
}

void Room306::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		release_trigger_on_digi_state(k10027, 1);
		break;

	case 3007:
		if (!_G(flags)[kPerkinsLostIsland]) {
			digi_preload_stream_breaks(SERIES2);
			series_stream_with_breaks(SERIES2, "306burnt", 6, 1, 1);
		} else if (_G(flags)[V118] == 3003) {
			digi_preload_stream_breaks(SERIES3);
			series_stream_with_breaks(SERIES3, "306burnt", 6, 1, 1);
		} else {
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "306fail", 6, 1, 1);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
