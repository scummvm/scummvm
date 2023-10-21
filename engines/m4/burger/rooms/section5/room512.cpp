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

#include "m4/burger/rooms/section5/room512.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room512::SERIES1[] = {
	{  0, "608_003",  3, 100, -1, 0, nullptr, 0 },
	{  1, "512f003a", 1, 255, -1, 0, &_state1, 11 },
	{  1, "512f003b", 1, 255, -1, 0, &_state1, 12 },
	{  1, "512f001a", 1, 255, -1, 0, &_state1, 21 },
	{  1, "512f001b", 1, 255, -1, 0, &_state1, 22 },
	{  1, "512f001c", 1, 255, -1, 0, &_state1, 23 },
	{  1, "512f002a", 1, 255, -1, 0, &_state1, 31 },
	{ 33, "608_001",  2, 200, -1, 0, nullptr,   0 },
	{ 48, "608_002",  2, 200, -1, 0, nullptr,   0 },
	STREAM_BREAK_END
};

int32 Room512::_state1;


Room512::Room512() : Section5Room() {
	_state1 = 0;
}

void Room512::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room512::init() {
	pal_cycle_init(101, 110, 6);
	kernel_trigger_dispatch_now(1);
}

void Room512::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		switch (_G(wilbur_should)) {
		case kCHANGE_WILBUR_ANIMATION:
			_state1 = imath_ranged_rand(11, 12);
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "512burnt", 6, 1, 2);
			break;

		case 5004:
			_state1 = 31;
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "512burnt", 6, 1, 2);
			break;

		default:
			_state1 = imath_ranged_rand(21, 23);
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "512burnt", 6, 1, 2);
			break;
		}
		break;

	case 2:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		release_trigger_on_digi_state(k10027, 1);
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
