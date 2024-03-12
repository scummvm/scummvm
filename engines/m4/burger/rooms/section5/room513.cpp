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

#include "m4/burger/rooms/section5/room513.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room513::SERIES1[] = {
	{   0, "513_001", 3,  40, -1, (uint)-1, nullptr, 0 },
	{   0, "513w001", 1, 255, -1,  0, nullptr, 0 },
	{  33, "513w002", 1, 255, -1,  0, nullptr, 0 },
	{  44, "513z001", 1, 255, -1,  0, nullptr, 0 },
	{  51, "513w003", 1, 255, -1,  0, nullptr, 0 },
	{  67, "513z002", 1, 255, -1,  0, nullptr, 0 },
	{ 117, "513z003", 1, 255, -1,  0, nullptr, 0 },
	{ 196, "513w005", 1, 255, -1,  0, nullptr, 0 },
	{ 209, "513z004", 1, 255, -1,  0, nullptr, 0 },
	{ 244, "513w006", 1, 255, -1,  0, nullptr, 0 },
	{ 292, "513z005", 1, 255, -1,  0, nullptr, 0 },
	{ 310, "513z006", 1, 255, -1,  0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room513::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room513::init() {
	_G(flags)[kFourthTestPassed] = 1;
	kernel_trigger_dispatch_now(1);
}

void Room513::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "513OUTRO", 6, 1, k10027);
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
