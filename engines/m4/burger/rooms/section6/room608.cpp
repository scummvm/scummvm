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

#include "m4/burger/rooms/section6/room608.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room608::SERIES1[] = {
	{  0, "608_003",  3, 100, -1, 0, nullptr, 0 },
	{  4, "608f004a", 1, 255, -1, 0, &_state1, 0 },
	{ 33, "608_001",  2, 255, -1, 0, nullptr, 0 },
	{ 48, "608_002",  2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room608::SERIES2[] = {
	{ 0, "608_003",  3, 100, -1, 0, nullptr,  0 },
	{ 4, "608f003a", 1, 255, -1, 0, &_state1, 0 },
	{ 4, "608f001",  1, 255, -1, 0, &_state1, 1 },
	{ 4, "608f002a", 1, 255, -1, 0, &_state1, 2 },
	{ 33, "608_001", 2, 255, -1, 0, nullptr,  0 },
	{ 48, "608_002", 2, 255, -1, 0, nullptr,  0 },
	STREAM_BREAK_END
};

int32 Room608::_state1;

Room608::Room608() : Section6Room() {
	_state1 = 0;
}

void Room608::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room608::init() {
	pal_cycle_init(101, 110, 6, -1, -1);
	_G(wilbur_should) = (_G(game).previous_room == 605 ||
		_G(game).previous_room == 606) ? 10016 : 10015;

	kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
}

void Room608::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		release_trigger_on_digi_state(10027, 1);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 10015:
			term_message("failed normally");
			_state1 = 0;
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "608fail", 6, 1, 1);
			break;

		case 10016:
			if (_G(game).previous_room == 606) {
				_state1 = 0;
			} else if (player_been_here(608)) {
				_state1 = 1;
			} else {
				_state1 = 2;
			}

			term_message("failed burnt");
			digi_preload_stream_breaks(SERIES2);
			series_stream_with_breaks(SERIES2, "608burnt", 6, 1, 1);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
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
