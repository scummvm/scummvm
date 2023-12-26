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

#include "m4/burger/rooms/section6/room609.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room609::SERIES1[] = {
	{  0, "609_001",  3, 125, -1, 0, nullptr,  0 },
	{ 20, "609w001",  1, 255, -1, 0, &_state1, 1 },
	{ 30, "609w002a", 1, 255, -1, 0, &_state1, 2 },
	{ 30, "609w002b", 1, 255, -1, 0, &_state1, 3 },
	{ 70, "609_002",  1, 255, -1, 0, nullptr,  0 },
	STREAM_BREAK_END
};

int32 Room609::_state1;

Room609::Room609() : Section6Room() {
	_state1 = 0;
}

void Room609::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room609::init() {
	_G(flags)[V277] = 6001;
	_G(flags)[kHampsterState] = 6007;
	kernel_trigger_dispatch_now(1);
}

void Room609::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		_state1 = _G(player).been_here_before ? 1 : imath_ranged_rand(2, 3);
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "609wi01", 6, 1, 6010);
		break;

	case 6010:
		_G(game).new_room = 612;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
