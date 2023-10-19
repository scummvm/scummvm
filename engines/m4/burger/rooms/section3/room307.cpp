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

#include "m4/burger/rooms/section3/room307.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room307::SERIES1[] = {
	{ 0, "307z001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room307::PLAY1[] = {
	{ 0,  5, "307z002w", 1, 255, -1, 2052, -1, nullptr, 0 },
	{ 0,  0, nullptr,    0,   0, -1,    0,  2, nullptr, 0 },
	{ 0,  5, "307z002x", 1, 255, -1, 2052, -1, nullptr, 0 },
	{ 0,  0, nullptr,    0,   0, -1,    0,  2, nullptr, 0 },
	{ 0,  5, "307z002y", 1, 255, -1, 2052, -1, nullptr, 0 },
	{ 0,  0, nullptr,    0,   0, -1,    0,  0, nullptr, 0 },
	{ 0,  5, "307z002z", 1, 255, -1, 2052, -1, nullptr, 0 },
	{ 6, -1, nullptr,    0,   0, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room307::init() {
	_G(flags)[kThirdTestPassed] = 1;
	ws_demand_location(418, 309, 9);
	ws_hide_walker();
	digi_preload("307_001");
	digi_play_loop("307_001", 3, 125, -1, 307);
	kernel_trigger_dispatch_now(1);
}

void Room307::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "307brig", 6, 1, 2);
		break;

	case 2:
		digi_unload_stream_breaks(SERIES1);
		_series1.show("307zg01", 0x400);
		ws_unhide_walker();
		kernel_trigger_dispatch_now(3);
		break;

	case 3:
		wilbur_speech("307w001y", 4);
		break;

	case 4:
		wilbur_speech("307w001z", 7);
		break;

	case 5:
		digi_preload("608_002");
		series_play("307door", 9, 16, -1, 7);
		digi_play("608_002", 2, 200, 6);
		break;

	case 6:
		wilbur_speech("307w002", k10027);
		break;

	case 7:
		_series1.terminate();
		series_play_with_breaks(PLAY1, "307zg01", 0x400, 5, 3, 5, 100, 0, 0);
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
