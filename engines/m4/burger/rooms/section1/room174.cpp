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

#include "m4/burger/rooms/section1/room174.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "HALLWAY",     nullptr,   "174w001", nullptr   },
	{ "TOILET",      "174w002", "174w001", nullptr   },
	{ "TOILET LID",  "174w002", "174w001", nullptr   },
	{ "TOILET TANK", "174w002", "174w001", nullptr   },
	{ "FLUSH CHAIN", "174w002", "174w001", nullptr   },
	{ "TUB",         "174w004", "174w001", "174w005" },
	{ "WINDOW",      nullptr,   "174w001", "174w006" },
	{ "SHOWERHEAD",  "174w007", "174w008", nullptr   },
	{ "CLOSET",      "174w009", "174w010", "174w010" },
	{ "MIRROR",      "174w011", nullptr,   "174w011" },
	{ "LAXATIVE ",   "174w012", nullptr,   "174w014" },
	{ "MEDICATIONS", "174w015", "174w016", "174w017" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0,  1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0, 18, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0,  3, "507_003", 2, 255, -1, 1, 5, nullptr, 0 },
	{ 4, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 6,  6, nullptr,   0,   0, -1, 0, 5, nullptr, 0 },
	{ 5,  5, nullptr,   0,   0, -1, 0, 5, nullptr, 0 },
	{ 4,  4, nullptr,   0,   0, -1, 0, 5, nullptr, 0 },
	PLAY_BREAK_END
};


void Room174::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 175:
		ws_demand_location(639, 299, 9);
		ws_walk(472, 297, nullptr, -1);
		break;

	default:
		ws_demand_location(252, 304, 5);
		break;
	}

	_series1 = series_show("174tlt01", 0x8ff);
	_series2 = series_show("174flush", 0xf00);

	if (inv_player_has("LAXATIVE")) {
		hotspot_set_active("LAXATIVE ", false);
	} else {
		_laxative.show("174wi01", 0x400, 0, -1, -1, 19);
	}
}

void Room174::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		switch (_action) {
		case 5:
			terminateMachineAndNull(_series1);
			_action = 6;
			series_play_with_breaks(PLAY3, "174tlt01", 0x8ff, 1, 2);
			break;

		case 6:
			player_set_commands_allowed(true);
			_series1 = series_show("174tlt01", 0x8ff);
			break;

		default:
			term_message("ERROR!!!!! toilet_should not set!");
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			disable_player();
			terminateMachineAndNull(_series2);
			_action = 5;
			_G(wilbur_should) = 2;
			series_play_with_breaks(PLAY1, "174wi03", 0x8ff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			ws_unhide_walker();
			wilbur_speech("174w003");
			_series2 = series_show("174flush", 0xf00);
			break;

		case 3:
			_laxative.terminate();
			ws_hide_walker();
			_G(wilbur_should) = 4;
			series_play_with_breaks(PLAY2, "174wi01", 0x400, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			enable_player();
			inv_give_to_player("LAXATIVE");
			hotspot_set_active("LAXATIVE ", false);
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

void Room174::pre_parser() {
	if (player_said("HALLWAY") && !player_said_any("GEAR", "LOOK AT"))
		player_set_facing_at(522, 287);
}

void Room174::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("HALLWAY") && player_said_any("GEAR", "LOOK AT")) {
		disable_player_commands_and_fade_init(1022);
	} else if (player_said("GEAR") && player_said_any("TOILET", "FLUSH CHAIN")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "LAXATIVE ")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		wilbur_speech("174w013", kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "MIRROR")) {
		wilbur_speech("174w001");
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
