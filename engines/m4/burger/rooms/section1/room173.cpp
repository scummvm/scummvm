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

#include "m4/burger/rooms/section1/room173.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "STAIRS",          "173w002", "173w003", nullptr   },
	{ "WRINGER",         "173w009", "173w010", "173w008" },
	{ "BREAKER BOX",     "504w017", nullptr,   nullptr   },
	{ "WASHING MACHINE", "173w007", nullptr,   "173w008" },
	{ "SHORTS",          "173w004", "173w005", "173w005" },
	{ "FAN BOY TOWEL",   "173w004", "173w005", "173w005" },
	{ "SOCK",            "173w004", "173w005", "173w005" },
	{ "T-SHIRT",         "173w004", "173w005", "173w005" },
	{ "LAUNDRY",         "173w004", "173w006", "173w006" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0,  5, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 0,  1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room173::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 172:
		ws_demand_location(136, 318, 5);
		_G(wilbur_should) = player_been_here(173) ? 1 : 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(332, 333, 5);
		break;
	}
}

void Room173::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		disable_player_commands_and_fade_init(1019);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			disable_player();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY1, "173wi01", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			disable_player();
			_G(wilbur_should) = 3;
			series_play_with_breaks(PLAY1, "173wi01", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			enable_player();
			wilbur_speech("173w001");
			break;

		case 4:
			disable_player();
			series_play_with_breaks(PLAY2, "173wi02", 0x200, -1, 3);
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

void Room173::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "STAIRS")) {
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
