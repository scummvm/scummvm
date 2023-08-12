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

#include "m4/burger/rooms/section1/room135.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID1[][4] = {
	{ "OLD BRIDGE",       "134W001", "134W002", "134W002" },
	{ "NEW BRIDGE",       "134W003", "134W002", "134W002" },
	{ "WATER",            "134W004", "134W002", "134W005" },
	{ "BANK",             "134W004", "134W002", "134W005" },
	{ "WRECKED TRUCK",    nullptr,   "134W002", "134w002" },
	{ "FORK IN THE ROAD", nullptr,   "134W002", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const char *SAID2[][4] = {
	{ "ODIE",             "135W001", "135W002", "135W002" },
	{ "OLD BRIDGE",       "135W005", "135W002", "135W002" },
	{ "NEW BRIDGE",       "135W006", "135W002", "135W002" },
	{ "WATER",            "135W007", "135W002", "135W008" },
	{ "FORK IN THE ROAD", nullptr,   "135W002", nullptr },
	{ "MAIN STREET",      nullptr,   "135W002", nullptr },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room135::init() {
	_G(player).walker_in_this_scene = true;
	_val1 = 255;
	_val2 = 0;
	_odieLoaded = false;
	_val4 = 1000;
	_val5 = 1;
	_val6 = 1;

	digi_preload("135_005");
	digi_preload("135_002");
	digi_preload("135_004");

	switch (_G(game).previous_room) {
	case -2:
		break;

	case 101:
		if (_G(flags)[V000] == 1002)
			_G(flags)[V039] = 1;

		_G(roomVal1) = 37;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	case 133:
	case 136:
		if (_G(flags)[V000] == 1002)
			_G(flags)[V039] = 1;

		_G(roomVal1) = 38;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	case 135:
		player_set_commands_allowed(false);
		kernel_trigger_dispatch_now(24);
		_val6 = 22;
		series_play_with_breaks(PLAY1, "134bu01");
		ws_demand_location(340, 250);
		ws_demand_facing(11);
		break;

	default:
		ws_demand_location(320, 271);
		ws_demand_facing(5);
		break;
	}

	if (_G(flags)[V000] == 1002) {
		hotspot_set_active("odie", false);
		hotspot_set_active("baitbox", false);
	} else {
		hotspot_set_active("wrecked truck", false);
	}

	_G(global_sound_room) = 135;
	_series1 = series_play("135cw01", 3840, 0, -1, 10, -1, 100, 0, 0, 0, 3);

	if (!_G(flags)[V039])
		kernel_trigger_dispatch_now(32);

	digi_preload("135_001");
	digi_play_loop("135_001", 3, 90, -1);

	if (_G(flags)[V000] == 1002) {
		_val7 = 27;
		_val8 = 27;
		kernel_trigger_dispatch_now(34);
		kernel_timing_trigger(imath_ranged_rand(200, 500), 21);
	}
}

void Room135::daemon() {
}

void Room135::pre_parser() {
	if (player_said("fork in the road") && player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9);

	if (player_said("main street") && player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	if (player_said("take", "baitbox") || player_said("gear", "baitbox"))
		player_hotspot_walk_override(308, 249, 10, -1);
}

void Room135::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if ((_G(flags)[V000] == 1002 && !_G(walker).wilbur_said(SAID1)) ||
			(_G(flags)[V000] != 1002 && !_G(walker).wilbur_said(SAID2))) {
		if (player_said("ENTER", "FORK IN THE ROAD") || player_said("gear", "fork in the road") ||
				(lookFlag && player_said("fork in the road"))) {
			player_set_commands_allowed(false);
			pal_fade_init(1009);

		} else if (player_said("ENTER", "MAIN STREET") || player_said("gear", "main street") ||
				(lookFlag && player_said("main street"))) {
			pal_fade_init(1001);
		} else if (player_said("conv01")) {
			conv01();
		} else if (player_said("conv02")) {
			conv02();
		} else if (player_said("conv03")) {
			conv03();
		} else if (player_said("odie") && inv_player_has(_G(player).verb)) {
			ws_hide_walker();
			player_set_commands_allowed(false);
			loadOdie();
			_val9 = 31;
			_val10 = 30;
			kernel_trigger_dispatch_now(33);
		} else if (inv_player_has(_G(player).verb) && player_said_any("fork in the road", "main street")) {
			_G(walker).wilbur_speech("135w002");
		} else if (lookFlag && player_said("baitbox")) {
			_G(walker).wilbur_speech(_G(flags)[V038] ? "135w004" : "135w003");
		} else if (lookFlag && player_said("wrecked truck")) {
			_G(walker).wilbur_speech(_G(flags)[V038] ? "135w007" : "135w006");
		} else if (player_said("take", "baitbox") || player_said("gear", "baitbox")) {
			player_set_commands_allowed(false);
			loadOdie();
			_val9 = 34;
			_val10 = 30;
			kernel_trigger_dispatch_now(33);
		} else if (player_said("talk to", "odie")) {
			player_set_commands_allowed(false);
			loadOdie();
			_flag1 = true;
			conv_load_and_prepare("conv03", 31);
			conv_export_pointer_curr(&_G(flags)[V038], 1);
			conv_export_value_curr(_G(flags)[V001], 3);
			conv_play_curr();
		} else if (player_said("conv06")) {
			conv03();
		}
	}

	_G(player).command_ready = false;
}

void Room135::conv01() {
	error("TODO: conv01");
}

void Room135::conv02() {
	error("TODO: conv02");
}

void Room135::conv03() {
	error("TODO: conv03");
}

void Room135::loadOdie() {
	static const char *NAMES[30] = {
		"135od05", "135od05s", "135od06", "135od06s", "135od04",
		"135od04s", "135od08", "135od08s", "135od09", "135od09s",
		"135od10", "135od10s", "135od11", "135od11s", "135od12",
		"135od12s", "135od13", "135od13s", "135od14", "135od14s",
		"135od16", "135od16s", "135od17", "135od17s", "135od18",
		"135od18s", "135od20", "135od20s", "135od21", "135od21s"
	};

	if (!_odieLoaded) {
		_odieLoaded = true;

		for (int i = 0; i < 30; ++i)
			series_load(NAMES[i], -1);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
