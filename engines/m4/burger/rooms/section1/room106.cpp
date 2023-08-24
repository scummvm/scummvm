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

#include "m4/burger/rooms/section1/room106.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "GARBAGE CANS",   "106w002", "106w003",  nullptr    },
	{ "POLICE STATION", "106w004", "106w004z", "106w004z" },
	{ "JAIL CELL",      "106w005", "106w004z", "106w004z" },
	{ "BARRED WINDOW",  "106w006", "106w004z", nullptr    },
	{ "FENCE ",         "106w008", "106w004z", "106w009"  },
	{ "RAZOR WIRE",     "106w010", "106w011",  "106w011"  },
	{ "GATE",           "106w012", "106w004z", "106w013"  },
	{ "SIGN",           "106w015", nullptr,    nullptr    },
	{ "TIRE",           "106w016", "106w017",  "106w018"  },
	{ "TOW TRUCK",      "106w019", "106w004z", "106w020"  },
	{ "PEGLEG",         "106w021", "106w022",  "106w022"  },
	{ "PEGLEG ",        "106w021", "106w022",  "106w022"  },
	{ "TIRE ",          "106w023", "106w020",  "106w020"  },
	{ "MAIN STREET",    nullptr,   "106w004z", nullptr    },
	{ "DOG COLLAR ",    "106w900", nullptr,    "106w901"  },
	{ "DOG COLLAR  ",   "106w900", nullptr,    "106w901"  },
	{ "HOOK ",          "106w903", "106w904",  "106w004z" },
	{ "WINCH",          "106w902", "106w004z", "106w904"  },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room106::PLAY1[] = {
	{ 0, -1, "106d003a", 2, 255, -1, 0, 0, &_val3, 1 },
	{ 0, -1, "106d003b", 2, 255, -1, 0, 0, &_val3, 2 },
	{ 0, -1, "106d003c", 2, 255, -1, 0, 0, &_val3, 3 },
	{ 0, -1, "106d003d", 2, 255, -1, 0, 0, &_val3, 4 },
	{ 0, -1, "106d003e", 2, 255, -1, 0, 0, &_val3, 5 },
	{ 0, -1, "106d003f", 2, 255, -1, 0, 0, &_val3, 6 },
	{ 0, -1, "106d003g", 2, 255, -1, 0, 0, &_val3, 7 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room106::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room106::PLAY3[] = {
	{ 0, -1, "106d001a", 2, 255, -1, 0, -1, &_val3, 1 },
	{ 0, -1, "106d001b", 2, 255, -1, 0, -1, &_val3, 2 },
	{ 0, -1, "106d001c", 2, 255, -1, 0, -1, &_val3, 3 },
	{ 0, -1, "106d001d", 2, 255, -1, 0, -1, &_val3, 4 },
	{ 0, -1, "106d001e", 2, 255, -1, 0, -1, &_val3, 5 },
	{ 0, -1, "106d001f", 2, 255, -1, 0, -1, &_val3, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room106::PLAY4[] = {
	{ 0, -1, "106d002a", 2, 255, -1, 0, -1, &_val3, 1 },
	{ 0, -1, "106d002b", 2, 255, -1, 0, -1, &_val3, 2 },
	{ 0, -1, "106d002c", 2, 255, -1, 0, -1, &_val3, 3 },
	{ 0, -1, "106d002d", 2, 255, -1, 0, -1, &_val3, 4 },
	{ 0, -1, "106d002e", 2, 255, -1, 0, -1, &_val3, 5 },
	{ 0, -1, "106d002f", 2, 255, -1, 0, -1, &_val3, 6 },
	{ 0, -1, "106d002g", 2, 255, -1, 0, -1, &_val3, 7 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room106::PLAY5[] = {
	{ 0,  4, "106d006a", 2, 255, -1, 0, 0, &_val3,  1 },
	{ 0,  4, "106d006b", 2, 255, -1, 0, 0, &_val3,  2 },
	{ 0,  4, "106d006c", 2, 255, -1, 0, 0, &_val3,  3 },
	{ 5, -1, "106_005",  2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room106::PLAY6[] = {
	{ 0, 4, "106_005",   2, 255, -1, 0, 0, nullptr, 0 },
	{ 5, -1, "106d007a", 2, 255, -1, 0, 0, &_val3,  1 },
	{ 5, -1, "106d007b", 2, 255, -1, 0, 0, &_val3,  2 },
	{ 5, -1, "106d007c", 2, 255, -1, 0, 0, &_val3,  3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room106::PLAY7[] = {
	{ 0, -1, "106d005",  2, 255, -1, 0, 0, &_val3, 1 },
	{ 0, -1, "106d005b", 2, 255, -1, 0, 0, &_val3, 2 },
	{ 0, -1, "106d005c", 2, 255, -1, 0, 0, &_val3, 3 },
	{ 0, -1, "106d005d", 2, 255, -1, 0, 0, &_val3, 4 },
	{ 0, -1, "106d005e", 2, 255, -1, 0, 0, &_val3, 5 },
	{ 0, -1, "106d005f", 2, 255, -1, 0, 0, &_val3, 6 },
	PLAY_BREAK_END
};

int Room106::_val3;

void Room106::preload() {
	_G(player).walker_in_this_scene = _G(game).room_id != 137 &&
		_G(game).room_id != 138;
}

void Room106::init() {
	_val3 = 0;

	_MATCH.clear();
	_MATCH.push_back(WilburMatch("JAWZ O' LIFE", "GATE", 2, nullptr, 0, &_val1, 2));
	_MATCH.push_back(WilburMatch("JAWZ O' LIFE", "CHAIN", 2, nullptr, 0, &_val1, 2));
	_MATCH.push_back(WilburMatch("JAWZ O' LIFE", "LOCK", 2, nullptr, 0, &_val1, 2));

	digi_preload("100_001");
	digi_preload("106_101");
	_digi1 = "100_001";

	kernel_trigger_dispatch_now(1);
	player_set_commands_allowed(false);
	_val1 = 0;

	setHotspots();
	loadSeries();

	if (_G(game).previous_room == -1) {
		_val2 = _G(flags)[V172] == 10023 ? 9 : 15;
	} else {
		_val2 = imath_ranged_rand(1, 2) == 1 ? 5 : 6;
	}

	kernel_trigger_dispatch_now(3);
	series_show("106gate", 0x4fd);
	series_show("106gates", 0x4fe);
	series_show("106tt", 0x600);
	series_show("106tire", 0x600);

	switch (_G(game).previous_room) {
	case 101:
		ws_demand_location(-40, 317, 3);
		ws_walk(62, 340, nullptr, 0);
		break;

	case 138:
		digi_play("106_102", 1, 255, 4);
		break;

	case RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	default:
		ws_demand_location(169, 336, 5);
		break;
	}
}

void Room106::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		_digi1 = !strcmp(_digi1, "106_101") ? "100_001" : "106_101";
		digi_play(_digi1, 3, 255, 1);
		break;

	case 2:
		switch (_val1) {
		case 1:
			player_set_commands_allowed(true);
			_G(walker).wilbur_speech_random("106w001a", "106w001b", "106w001c");
			break;
		case 2:
			wilbur_speech("106w014");
			break;
		case 3:
			wilbur_speech("106w501", 5);
			break;
		case 4:
			wilbur_speech("106w500", 10022);
			break;
		default:
			break;
		}
		break;

	case 3:
		switch (_val2) {
		case 5:
			_val3 = imath_ranged_rand(1, 7);
			_val2 = 7;
			series_play_with_breaks(PLAY1, "106dg01", 0x501, 3, 3);
			break;

		case 6:
			_val2 = 7;
			series_play_with_breaks(PLAY2, "106dg02", 0x501, 3, 3);
			break;

		case 7:
			_val3 = imath_ranged_rand(1, 6);
			_val2 = _G(game).previous_room == 137 || _G(game).previous_room == 138 ? 9 : 8;
			series_play_with_breaks(PLAY3, "106dg04", 0x501, 3, 3);
			break;

		case 8:
			_val1 = 1;
			ws_turn_to_face(calc_facing(415, 234), 2);
			_val2 = 9;
			kernel_trigger_dispatch_now(3);
			break;

		case 9:
			_G(flags)[V172] = 10023;

			switch (imath_ranged_rand(1, 6)) {
			case 1:
				_val2 = 10;
				break;
			case 2:
				_val2 = 11;
				break;
			case 3:
				_val2 = 12;
				break;
			default:
				_val2 = 9;
				break;
			}

			series_show("106dg07", 0x501, 0, 3, 120);
			series_show("106dg07s", 0x502, 0, -1, 120);
			break;

		case 10:
			if (digi_play_state(1)) {
				_val2 = 9;
				kernel_trigger_dispatch_now(3);
			} else {
				_val3 = imath_ranged_rand(1, 7);
				_val2 = 9;
				series_play_with_breaks(PLAY4, "106dg05", 0x501, 3, 3);
			}
			break;

		case 11:
			if (digi_play_state(1)) {
				_val2 = 9;
				kernel_trigger_dispatch_now(3);
			} else {
				_val3 = imath_ranged_rand(1, 3);
				_val2 = 15;
				series_play_with_breaks(PLAY5, "106dg07", 0x501, 3, 3);
			}
			break;

		case 12:
			if (digi_play_state(1)) {
				_val2 = 9;
				kernel_trigger_dispatch_now(3);
			} else {
				_val3 = imath_ranged_rand(1, 6);
				_val2 = 9;
				series_play_with_breaks(PLAY3, "106dg04", 0x501, 3, 3);
			}
			break;

		case 13:
			if (digi_play_state(1)) {
				_val2 = 15;
				kernel_trigger_dispatch_now(3);
			} else {
				_val3 = imath_ranged_rand(1, 6);
				_val2 = 15;
				series_play_with_breaks(PLAY3, "106dg09", 0x501, 3, 3);
			}
			break;

		case 14:
			if (digi_play_state(1)) {
				_val2 = 15;
				kernel_trigger_dispatch_now(3);
			} else {
				_val3 = imath_ranged_rand(1, 3);
				_val2 = 9;
				series_play_with_breaks(PLAY6, "106dg11", 0x501, 3, 3);
			}
			break;

		case 15:
			_G(flags)[V172] = 10024;

			switch (imath_ranged_rand(1, 4)) {
			case 1:
				_val2 = 13;
				break;
			case 2:
				_val2 = 14;
				break;
			default:
				_val2 = 15;
				break;
			}

			series_show("106dg07", 0x501, 0, 3, 120, 8);
			series_show("106dg07s", 0x502, 0, -1, 120, 8);
			break;

		case 16:
			_val3 = imath_ranged_rand(1, 6);
			_val2 = 11;
			series_play_with_breaks(PLAY7, "106dg06", 0x501, 3, 3);
			break;

		case 17:
			_val3 = imath_ranged_rand(1, 6);
			_val2 = 13;
			series_play_with_breaks(PLAY7, "106dg10", 0x501, 3, 3);
			break;

		default:
			break;
		}
		break;

	case 4:
		_val1 = 3;
		digi_play("106s001", 1, 255, 2);
		break;

	case 5:
		_val1 = 4;
		digi_play("106_103", 1, 255, 2);
		break;

	case 10008:
		_val2 = _G(flags)[V172] == 10023 ? 16 : 17;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room106::pre_parser() {
	if (player_said("SKY"))
		player_set_facing_hotspot();

	if (player_said("MAIN STREET") && player_said_any("GEAR", "LOOK AT")) {
		player_set_facing_at(-40, 317);
	}
}

void Room106::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(walker).wilbur_said(SAID)) {
		if (player_said_any("GEAR", "LOOK AT") && player_said("MAIN STREET")) {
			disable_player_commands_and_fade_init(1001);
			_G(player).command_ready = false;
		} else if (_G(walker).wilbur_match(_MATCH)) {
			_G(player).command_ready = false;
		}
	}
}

void Room106::setHotspots() {
	hotspot_set_active("PEGLEG", false);
	hotspot_set_active("PEGLEG ", false);
	hotspot_set_active("DOG COLLAR ", false);
	hotspot_set_active("DOG COLLAR  ", false);

	switch (_G(flags)[V172]) {
	case 10023:
		hotspot_set_active("PEGLEG", true);
		hotspot_set_active("DOG COLLAR ", true);
		break;

	case 10024:
		hotspot_set_active("PEGLEG ", true);
		hotspot_set_active("DOG COLLAR  ", true);
		break;

	default:
		break;
	}
}

void Room106::loadSeries() {
	series_load("106dg04");
	series_load("106dg04s");
	series_load("106dg07");
	series_load("106dg07s");
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
