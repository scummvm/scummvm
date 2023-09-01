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

#include "m4/burger/rooms/section6/room601.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak SERIES1[] = {
	{   3, "601Z001",  1, 255, -1, 0, nullptr, 0 },
	{  58, "601Z003",  1, 255, -1, 0, nullptr, 0 },
	{ 109, "601_002",  2, 255, -1, 0, nullptr, 0 },
	{ 142, "601_003",  2, 255, -1, 0, nullptr, 0 },
	{ 154, "601_004",  2, 255, -1, 0, nullptr, 0 },
	{ 155, "601F001",  1, 255, -1, 0, nullptr, 0 },
	{ 208, "601W001",  1, 255, -1, 0, nullptr, 0 },
	{ 224, "601Z004",  1, 255, -1, 0, nullptr, 0 },
	{ 264, "601F003",  1, 255, -1, 0, nullptr, 0 },
	{ 304, "601Z005",  1, 255, -1, 0, nullptr, 0 },
	{ 432, "601W002",  1, 255, -1, 0, nullptr, 0 },
	{ 462, "601Z006",  1, 255, -1, 0, nullptr, 0 },
	{ 492, "601Z007",  1, 255, -1, 0, nullptr, 0 },
	{ 528, "601_009a", 2, 255, -1, 0, nullptr, 0 },
	{ 543, "601_009b", 2, 255, -1, 0, nullptr, 0 },
	{ 556, "601_009c", 2, 255, -1, 0, nullptr, 0 },
	{ 566, "601_005",  2, 255, -1, 0, nullptr, 0 },
	{ 580, "601_006",  2, 255, -1, 0, nullptr, 0 },
	{ 679, "601_010",  1, 255, -1, 0, nullptr, 0 },
	{ 689, "601_007",  2, 255, -1, 0, nullptr, 0 },
	{ 695, "601_011",  1, 255, -1, 0, nullptr, 0 },
	{ 706, "601_010",  2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room601::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room601::init() {
	_G(flags).reset2();
	digi_preload("601_001");
	digi_play_loop("601_001", 3, 40);
	digi_preload_stream_breaks(SERIES1);
	kernel_trigger_dispatch_now(1);

	_G(kernel).suppress_fadeup = true;
	pal_fade_set_start(0);
	pal_fade_init(_G(kernel).first_fade, 255, 100, 30, -1);
	pal_cycle_init(118, 127, 6, -1, -1);
}

void Room601::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		series_stream_with_breaks(SERIES1, "601lab", 6, 1, 6004);
		_G(kernel).suppress_fadeup = false;
		break;

	case 6004:
		_G(game).new_room = 604;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
