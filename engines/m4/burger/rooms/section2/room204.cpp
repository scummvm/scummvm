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

#include "m4/burger/rooms/section2/room204.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesPlayBreak Room204::PLAY1[] = {
	{ 0, 2, 0, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room204::PLAY2[] = {
	{  2,  8, nullptr,   0,  0, -1, 0, 0, nullptr, 0 },
	{  9, 14, "204_001", 2, 63, -1, 0, 0, nullptr, 0 },
	{ 14,  9, "204_001", 2, 63, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room204::PLAY3[] = {
	{ 15, 18, "204w001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room204::PLAY4[] = {
	{ 18, -1, nullptr, 0, 0, -1, 16, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room204::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room204::init() {
	digi_preload("200_001");
	digi_play_loop("200_001", 3, 85, -1, 200);
	series_load("204as01");
	series_load("204as01s");
	digi_preload("204_001");
	digi_preload("204w001");

	kernel_trigger_dispatch_now(1);
}

void Room204::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		Series::series_show("204as01", 0x100, 0, 2, 120, 0);
		break;

	case 2:
		series_play_with_breaks(PLAY1, "204as01", 0x100, 3, 1);
		break;

	case 3:
		Series::series_show("204as01", 0x100, 0, 4, 60, 2);
		break;

	case 4:
		series_play_with_breaks(PLAY2, "204as01", 0x100, 5, 1, 6);
		break;

	case 5:
		Series::series_show("204as01", 0x100, 0, 6, 60, 14);
		break;

	case 6:
		series_play_with_breaks(PLAY3, "204as01", 0x100, 7, 1, 6);
		break;

	case 7:
		Series::series_show("204as01", 0x100, 0, 8, 15, 18);
		break;

	case 8:
		series_play_with_breaks(PLAY4, "204as01", 0x100, 9, 1, 6);
		break;

	case 9:
		kernel_timing_trigger(60, 10);
		break;

	case 10:
		disable_player_commands_and_fade_init(2002);
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
