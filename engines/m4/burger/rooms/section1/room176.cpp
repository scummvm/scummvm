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

#include "m4/burger/rooms/section1/room176.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "HALLWAY",        nullptr,   "176w001", nullptr   },
	{ "PUZ DISPENSER ", "176w002", nullptr,   "176w003" },
	{ "ROBOT",          "176w004", "176w005", "176w006" },
	{ "WINDOW",         nullptr,   "176w001", "176w010" },
	{ "BED",            nullptr,   nullptr,   "176w016" },
	{ "COMIC BOOK",     "176w007", "176w008", "176w008" },
	{ "COMIC BOOK ",    nullptr,   "176w008", "176w008" },
	{ "GERBIL CAGE",    "176w009", "176w005", "176w005" },
	{ "TELEVISION",     "176w011", "176w015", "176w015" },
	{ "WARDROBE",       "176w017", nullptr,   "176w001" },
	{ "DRESSER",        "176w018", nullptr,   "176w001" },
	{ "BOOKS",          nullptr,   "176w008", "176w008" },
	{ "MODEL ROCKET",   "176w006", "176w005", "176w006" },
	{ "MODEL ROCKET ",  "176w019", "176w005", "176w006" },
	{ "MODEL ROCKET  ", "176w020", "176w005", "176w006" },
	{ "POSTER",         "176w021", "176w005", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 22, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room176::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 175:
		ws_demand_location(546, 239, 9);
		ws_walk(447, 251, nullptr, -1);
		break;

	default:
		ws_demand_location(340, 279, 5);
		break;
	}

	if (inv_player_has("PUZ DISPENSER") || inv_player_has("BROKEN PUZ DISPENSER")) {
		hotspot_set_active("PUZ DISPENSER ", false);
	} else {
		_puzDispenser = series_show("176pez", 0x500);
	}
}

void Room176::daemon() {
	switch (_G(kernel).trigger) {
	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			disable_player();
			terminateMachineAndNull(_puzDispenser);
			_G(wilbur_should) = 2;
			series_play_with_breaks(PLAY1, "176wi01", 0x500, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			enable_player();
			inv_give_to_player("BROKEN PUZ DISPENSER");
			hotspot_set_active("PUZ DISPENSER ", false);
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

void Room176::pre_parser() {
	if (player_said("HALLWAY") && !player_said_any("GEAR", "LOOK AT"))
		player_set_facing_at(487, 238);
}

void Room176::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("HALLWAY")) {
		disable_player_commands_and_fade_init(1022);
	} else if (player_said("TAKE", "PUZ DISPENSER ")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
