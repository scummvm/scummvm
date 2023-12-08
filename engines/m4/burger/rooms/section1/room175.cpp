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

#include "m4/burger/rooms/section1/room175.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/core/play_break.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "WILBUR'S ROOM",     nullptr,   "175w001", nullptr   },
	{ "BATHROOM",          nullptr,   "175w001", nullptr   },
	{ "AUNT POLLY'S ROOM", "175w002", "175w001", "175w003" },
	{ "STAIRS",            "175w004", "175w001", nullptr   },
	{ "WINDOW",            nullptr,   nullptr,   "175w005" },
	{ "VASE",              "175w006", "175w007", "175w008" },
	{ "PICTURE",           "175w012", "175w010", "175w011" },
	{ "PICTURE ",          "175w013", "175w010", "175w011" },
	{ "PICTURE  ",         "175w014", "175w010", "175w011" },
	{ "PICTURE   ",        "175w009", "176w010", "175w011" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0,  3, "175_002a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 4, -1, "175_002b", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0,  2, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{ 3, -1, "175_002a", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};


void Room175::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 171:
		ws_demand_location(331, 272, 3);
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 174:
		ws_demand_location(150, 307, 3);
		ws_walk(222, 307, nullptr, -1);
		break;

	case 176:
		ws_demand_location(72, 348, 3);
		ws_walk(170, 349, nullptr, -1);
		break;

	default:
		ws_demand_location(327, 340, 5);
		break;
	}

	digi_preload("175_001");
	digi_play_loop("175_001", 2, 125);
	series_play("175fan", 0xf00, 0, -1, 6, -1);
}

void Room175::daemon() {
	switch (_G(kernel).trigger) {
	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			disable_player();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY1, "175wi01", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			disable_player();
			series_play_with_breaks(PLAY2, "175wi02", 0xc00, -1, 3);
			disable_player_commands_and_fade_init(1018);
			break;

		case 3:
			disable_player();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY3, "175wi03", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
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

void Room175::pre_parser() {
	if (!player_said_any("GEAR", "LOOK AT")) {
		if (player_said("BATHROOM"))
			player_set_facing_at(194, 301);

		if (player_said("WILBUR'S ROOM"))
			player_set_facing_at(126, 343);
	}
}

void Room175::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "STAIRS")) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("BATHROOM")) {
		disable_player_commands_and_fade_init(1021);
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("WILBUR'S ROOM")) {
		disable_player_commands_and_fade_init(1023);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
