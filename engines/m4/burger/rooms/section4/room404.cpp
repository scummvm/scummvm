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

#include "m4/burger/rooms/section4/room404.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room404::SAID[][4] = {
	{ "QUARTER ",     "404w001", nullptr,   "404w002" },
	{ "IMPOUND YARD", nullptr,   "400w001", nullptr   },
	{ "TOILET",       "404w003", "400w001", "404w004" },
	{ "SINK",         "404w005", "400w001", "404w006" },
	{ "COT",          "404w007", nullptr,   "404w008" },
	{ "MARKS",        "404w103", nullptr,   nullptr   },
	{ "NEWSPAPER",    "404w009", "404w010", "404w011" },
	{ "BARS",         "404w012", "400w001", "400w001" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room404::PLAY1[] = {
	{  1,  9, nullptr, 0, 0,    -1, 0, 0, nullptr, 0 },
	{ 10, 17, nullptr, 0, 0, 10016, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room404::init() {
	digi_preload("400_001");
	digi_play_loop("400_001", 3, 125);
	pal_cycle_init(112, 127, 6);

	if (_G(flags)[V162]) {
		hotspot_set_active("QUARTER ", false);
	} else {
		hotspot_set_active("QUARTER ", true);
		_quarter.show("404wi01", 0xf00);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 406:
		ws_demand_location(320, 350, 1);
		break;

	default:
		_G(flags)[V171] = 4001;
		_G(flags)[V172] = 10026;
		_G(flags)[V176] = 1;
		_G(flags)[V175] = 1;
		_G(flags)[V174] = 4004;
		ws_demand_location(320, 271, 5);
		break;
	}
}

void Room404::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_update_info();

		if (_G(player_info).y > 350) {
			disable_player_commands_and_fade_init(-1);
		} else {
			kernel_timing_trigger(6, 1);
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			_G(wilbur_should) = 2;
			disable_player();
			series_play_with_breaks(PLAY1, "404wi01", 0x200, kCHANGE_WILBUR_ANIMATION, 1);
			break;

		case 2:
			_G(wilbur_should) = 10001;
			_quarter.terminate();
			_G(flags)[V162] = 1;
			hotspot_set_active("QUARTER ", false);
			inv_give_to_player("QUARTER");
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

void Room404::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("IMPOUND YARD")) {
		if (!player_said_any("EXIT", "LOOK AT", "GEAR")) {
			player_hotspot_walk_override(320, 346, 5);
		} else {
			player_set_commands_allowed(false);
			kernel_trigger_dispatch_now(1);
		}
	}
}

void Room404::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("IMPOUND YARD")) {
		if (player_said_any("EXIT", "LOOK AT", "GEAR")) {
			kernel_trigger_dispatch_now(4005);
		} else {
			wilbur_speech("400w001");
		}
	} else if (player_said("TAKE", "QUARTER ")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TOILET")) {
		wilbur_speech("404w004");
	} else if (player_said("BARS")) {
		wilbur_speech("404w013");
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
