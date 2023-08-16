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

#include "m4/burger/rooms/section1/room142.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID1[][4] = {
	{ "VERA'S DINER",   "142w001", "142w002", "142w002" },
	{ "CAR",            "142w003", nullptr,   "142w004" },
	{ "FANBELT",        "142w005", nullptr,   "142w002" },
	{ "ICE BOX",        "142w006", "142w002", nullptr   },
	{ "ICE",            nullptr,   nullptr,   "142w002" },
	{ "GARBAGE CANS",   "142w010", "142w011", "142w012" },
	{ "BACK DOOR",      "142w013", "142w002", nullptr   },
	{ "PLANTS",         "142w014", "142w015", "142w002" },
	{ "SIGN",           "142w016", "142w017", "142w002" },
	{ "SIGN ",          "142w018", "142w002", "142w002" },
	{ "FRONT DOOR",     nullptr,   "142w002", nullptr   },
	{ "MAIN STREET",    nullptr,   "142w002", nullptr   },
	{ "HANLON'S POINT", nullptr,   "142w002", nullptr   },
	{ "AUNT POLLY'S HOUSE", nullptr, "142w002", nullptr },
	{ "PARKING LOT",    nullptr,   "142w002", nullptr   },
	{ "HIGHWAY 2",      "142w020", "142w002", "142w021" },
	{ nullptr, nullptr, nullptr, nullptr }
};

int Room142::_val1;
int Room142::_val2;

Room142::Room142() : Room() {
	_val1 = 0;
	_val2 = 0;

	_MATCH.push_back(WilburMatch("GEAR", "PARKING LOT", 9,
		&_G(flags)[V000], 1003, &_val1, 18));
	_MATCH.push_back(WilburMatch("LOOK AT", "PARKING LOT", 9,
		&_G(flags)[V000], 1003, &_val1, 18));
	_MATCH.push_back(WilburMatch("GEAR", "PARKING LOT", 5,
		&_G(flags)[V058], 0, &_val2, 13));
	_MATCH.push_back(WilburMatch("LOOK AT", "PARKING LOT", 5,
		&_G(flags)[V058], 0, &_val2, 13));
	_MATCH.push_back(WilburMatch("GEAR", "ICE BOX", gTELEPORT,
		&_G(flags)[V059], 0, &_G(roomVal1), 1));
	_MATCH.push_back(WilburMatch("TAKE", "FANBELT", gTELEPORT,
		nullptr, 0, &_G(roomVal1), 9));
	_MATCH.push_back(WilburMatch("GEAR", "BACK DOOR", 6,
		nullptr, 0, nullptr, 0));
}

void Room142::init() {
	_G(player).walker_in_this_scene = true;
	digi_preload("142_004");
	digi_play_loop("142_004", 3, 255, -1);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		if (_G(flags)[V059]) {
			ws_hide_walker();
			_G(roomVal1) = 2;
			kernel_trigger_dispatch_now(gTELEPORT);
		}
		break;

	case 101:
		ws_demand_location(120, 400, 2);
		ws_walk(120, 344, 0, -1, -1);
		break;

	case 139:
		ws_demand_location(-40, 375, 2);
		ws_walk(25, 344, 0, -1, -1);
		break;

	case 143:
		ws_demand_location(350, 270, 8);
		ws_walk(297, 275, 0, -1, -1);
		break;

	case 145:
		ws_demand_location(293, 275, 7);
		ws_walk(282, 280, 0, -1, -1);
		break;

	case 170:
		ws_demand_location(680, 325, 9);
		ws_walk(613, 331, 0, -1, -1);
		break;

	default:
		ws_demand_location(270, 320);
		ws_demand_facing(5);
		break;
	}

	series_play("142sm01", 0xf00, 4, -1, 6, -1);
	_series1 = series_show("142door", 0xe00);

	if (inv_player_has("PANTYHOSE")) {
		hotspot_set_active("FANBELT", false);
		hotspot_set_active("ENGINE", true);
	} else {
		hotspot_set_active("FANBELT", true);
		hotspot_set_active("ENGINE", false);
	}

	hotspot_set_active("TRUCK", false);

	if (_G(flags)[V000] == 1003) {
		_noWalk = intr_add_no_walk_rect(230, 250, 294, 277, 229, 278,
			_G(screenCodeBuff)->get_buffer());
		_series2 = series_show("142dt01", 0xd00, 0, -1, -1, 22);
		_series3 = series_show("142dt01s", 0xd01, 0, -1, -1, 22);
		hotspot_set_active("TRUCK", true);
	}

	hotspot_set_active("TOUR BUS", false);

	if (_G(flags)[V058]) {
		_series4 = series_show("142ba01", 0xf00, 0, -1, -1, 21);
		hotspot_set_active("TOUR BUS", true);
	}

	if (!_G(flags)[V059]) {
		_series5 = series_show("142icedr", 0xe00);
		hotspot_set_active("ICE", false);
	}
}

void Room142::daemon() {
}

void Room142::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V059]) {
		_G(player).ready_to_walk = false;
		_G(player).need_to_walk = false;

		if (player_said("GEAR", "ICE BOX")) {
			_G(roomVal1) = 5;
			kernel_trigger_dispatch_now(gTELEPORT);
		} else if (player_said("ICE")) {
			if (player_said("LOOK AT")) {
				if (_G(flags)[V040] && !inv_player_has("BLOCK OF ICE")) {
					_G(walker).wilbur_speech("142w008");
				} else {
					_G(walker).wilbur_speech("142w007");
				}
			} else if (player_said("TAKE")) {
				if (!_G(flags)[V040]) {
					_G(walker).wilbur_speech("142w017");
				} else if (inv_player_has("BLOCK_OF_ICE")) {
					_G(walker).wilbur_speech("142w009");
				} else {
					kernel_trigger_dispatch_now(gTELEPORT);
				}
			} else if (player_said("GEAR")) {
				_G(walker).wilbur_speech("142w002");
			} else {
				term_message("ERROR - don't know what to do with ice!!!");
			}
		} else {
			_G(roomVal1) = 7;
			kernel_trigger_dispatch_now(gTELEPORT);
		}

		_G(player).command_ready = false;

	} else {
		if (_G(flags)[V000] == 1003 &&
			_G(player).walk_x >= 230 && _G(player).walk_x <= 294 &&
			_G(player).walk_y >= 250 && _G(player).walk_y <= 277) {
			player_hotspot_walk_override(_G(hotspot_x), 278);
		} else if (player_said_any("GEAR", "LOOK AT")) {
			if (player_said("MAIN STREET")) {
				player_hotspot_walk_override(_G(player).walk_x, 400);
				checkAction();
				_G(kernel).call_daemon_every_loop = true;
				_G(player).command_ready = false;
			} else if (player_said("PARKING LOT") && _G(flags)[V058]) {
				checkAction();
				_G(kernel).call_daemon_every_loop = true;
				_G(player).command_ready = false;
			}

		} else if (player_said("MAIN STREET")) {
			player_set_facing_at(120, 400);
		} else if (player_said("HANLON'S POINT")) {
			player_set_facing_at(-40, 375);
		} else if (player_said("AUNT POLLY'S HOUSE")) {
			player_set_facing_at(680, 325);
		} else if (player_said("FRONT DOOR")) {
			player_set_facing_at(350, 270);
		} else if (player_said("HIGHWAY 2")) {
			player_set_facing_at(192, 252);
		} else if (player_said("PARKING LOT")) {
			player_set_facing_at(303, 247);
		} else if (player_said("TOUR BUS")) {
			checkAction();
			_G(kernel).call_daemon_every_loop = true;
			_G(player).command_ready = false;
		}
	}
}

void Room142::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(walker).wilbur_said(SAID1)) {
		if (player_said_any("GEAR", "LOOK AT") && player_said("HANLON'S POINT")) {
			disable_player_commands_and_fade_init(1012);
		} else if (player_said_any("GEAR", "LOOK AT") && player_said("FRONT DOOR")) {
			disable_player_commands_and_fade_init(1015);
		} else if (player_said_any("GEAR", "LOOK AT") && player_said("AUNT POLLY'S HOUSE")) {
			disable_player_commands_and_fade_init(1017);
		} else if (!_G(walker).wilbur_match(_MATCH)) {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room142::checkAction() {
	_actionType = 0;

	if (player_said_any("GEAR", "LOOK AT", "GO TO")) {
		if (player_said("MAIN STREET")) {
			_actionType = 1;
		} else if (_G(flags)[V058] && (player_said("PARKING LOT") ||
				player_said("GO TO", "TOUR BUS"))) {
			_actionType = 2;
		}
	}

	if (_actionType) {
		player_update_info();

		if (_actionType == 1 && _G(player_info).y > 375 && player_commands_allowed()) {
			player_update_info();
			player_hotspot_walk_override(_G(player_info).x, 400);
			disable_player_commands_and_fade_init(1001);

		} else if (_actionType == 2 && _G(player_info).y < 280 &&
				player_commands_allowed()) {
			disable_player_commands_and_fade_init(1016);
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
