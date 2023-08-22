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

#include "m4/burger/rooms/section1/room133_136.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "SIGN",         "136w001", "136W003", nullptr   },
	{ "SIGN ",        "136w002", "136W003", "136w005" },
	{ "CONSTRUCTION", nullptr,   "136W008", "136W008" },
	{ "OLD BRIDGE",   nullptr,   "136W008", nullptr   },
	{ "TOWN LIMITS",  nullptr,   "136W008", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

void Room133_136::init() {
	_G(player).walker_in_this_scene = true;
	_flag = true;
	_volume = 255;
	_walkCodes = nullptr;
	_G(kernel).call_daemon_every_loop = true;

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		break;

	case 134:
	case 135:
		_G(roomVal1) = 3;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	case 136:
		ws_demand_location(444, 230, 2);
		break;

	case 137:
		_G(roomVal1) = 4;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	setupSign();
	setupSignWalkAreas();
	digi_preload("136_001");
	digi_play_loop("136_001", 3, 180);
}

void Room133_136::daemon() {
}

void Room133_136::pre_parser() {
	if (player_said("gear", "sign") && !_G(flags)[V043])
		player_hotspot_walk_override(246, 247, 8);

	if (player_said("old bridge") &&
			player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	if (player_said("town limits") &&
		player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9);

	if (player_said("enter", "construction"))
		player_hotspot_walk_override(292, 173, 1);
}

void Room133_136::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("gear", "sign") && !_G(flags)[V043]) {
		kernel_trigger_dispatch_now(16);

	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled

	} else if (lookFlag && player_said("construction")) {
		wilbur_speech(_G(flags)[V000] == 1002 ? "136w006" : "136w007");

	} else if (player_said("enter", "old bridge") || player_said("gear", "old bridge") ||
			(lookFlag && player_said("old bridge"))) {
		player_set_commands_allowed(false);
		pal_fade_init(1008);

	} else if (player_said("enter", "town limits") || player_said("gear", "town limits") ||
			(lookFlag && player_said("town limits"))) {
		player_set_commands_allowed(false);
		pal_fade_init(1010);

	} else if (inv_player_has(_G(player).verb) &&
		player_said_any("old bridge", "town limits", "construction")) {
		wilbur_speech("136w008");
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room133_136::setupSign() {
	if (_G(flags)[V043]) {
		hotspot_set_active("sign", false);
		hotspot_set_active("sign ", true);
		_sign1 = series_play("136signr", 0x800, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_sign2 = series_play("136sinrs", 0x801, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	} else {
		hotspot_set_active("sign ", false);
		hotspot_set_active("sign", true);

		_sign1 = series_play("136sign1", 0xa00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_sign2 = series_play("136sin1s", 0xa01, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	}
}

void Room133_136::setupSignWalkAreas() {
	if (_G(flags)[V043]) {
		intr_add_no_walk_rect(294, 263, 332, 278, 297, 292);
		intr_add_no_walk_rect(332, 278, 380, 300, 297, 292);

		if (_walkCodes)
			intr_remove_no_walk_rect(_walkCodes);
	} else {
		_walkCodes = intr_add_no_walk_rect(144, 241, 244, 252, 167, 272);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
