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

#include "m4/burger/rooms/section6/room610.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room610::SERIES1[] = {
	{   1, "610_001", 3,  40, -1, 1024, nullptr, 0 },
	{   7, "610_002", 2, 175, -1,    0, nullptr, 0 },
	{   9, "610Z001", 1, 255, -1,    0, nullptr, 0 },
	{  10, nullptr,   2,   0, -1,    0, nullptr, 0 },
	{  11, "610_003", 2, 175, -1,    0, nullptr, 0 },
	{  14, nullptr,   2,   0, -1,    0, nullptr, 0 },
	{  62, "610Z002", 1, 255, -1,    0, nullptr, 0 },
	{ 139, "610Z003", 1, 255, -1,    0, nullptr, 0 },
	{ 145, "610_002", 2, 175, -1,    0, nullptr, 0 },
	{ 148, nullptr,   2,   0, -1,    0, nullptr, 0 },
	{ 149, "610_004", 2, 175, -1,    0, nullptr, 0 },
	{ 152, nullptr,   2,   0, -1,    0, nullptr, 0 },
	{ 166, "610_005", 2, 125, -1,    0, nullptr, 0 }, 
	{ 174, "610_006", 2,  70, -1,    0, nullptr, 0 },  
	{ 176, "610Z004", 1, 255, -1,    0, nullptr, 0 }, 
	{ 202, "610Z005", 1, 255, -1,    0, nullptr, 0 }, 
	{ 238, "610F001", 1, 255, -1,    0, nullptr, 0 }, 
	{ 277, "610Z006", 1, 255, -1,    0, nullptr, 0 }, 
	{ 285, "610F002", 1, 255, -1,    0, nullptr, 0 }, 
	{ 292, "610_006", 2,  70, -1,    0, nullptr, 0 },  
	{ 336, "610Z007", 1, 255, -1,    0, nullptr, 0 }, 
	{ 387, "610F003", 1, 255, -1,    0, nullptr, 0 }, 
	{ 400, "610_006", 2,  70, -1,    0, nullptr, 0 },  
	{ 414, "610Z008", 1, 255, -1,    0, nullptr, 0 }, 
	{ 447, "610F004", 1, 255, -1,    0, nullptr, 0 }, 
	{ 452, "610_007", 2,  75, -1,    0, nullptr, 0 },  
	{ 458, "610Z009", 1, 255, -1,    0, nullptr, 0 }, 
	{ 562, nullptr,   0,   0,  2,    0, nullptr, 0 },
	STREAM_BREAK_END
};

int32 Room610::_state1;

Room610::Room610() : Section6Room() {
	_state1 = 0;
}

void Room610::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room610::init() {
	_G(flags)[kSecondTestPassed] = 1;
	_G(kernel).suppress_fadeup = 1;
	kernel_trigger_dispatch_now(1);
}

void Room610::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_preload_stream_breaks(SERIES1);
		pal_fade_set_start(0);
		series_stream_with_breaks(SERIES1, "610A", 6, 1, 10027);
		pal_fade_init(_G(kernel).first_fade, 255, 100, 60, -1);
		break;

	case 2:
		pal_fade_init(0, 255, 0, 30, -1);
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
