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

#include "m4/burger/rooms/section5/room502.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const Section5Room::BorkPoint Room502::BORK_TABLE[] = {
	{ 386, 44 }, { 368, 62 }, { 343, 80 }, { 314, 109 }, { 298, 123 },
	{ 279, 143 }, { 255, 166 }, { 255, 166 }, { 252, 166 }, { 249, 168 },
	{ 252, 168 }, { 256, 168 }, { 254, 168 }, { 250, 167 }, { 248, 164 },
	{ 254, 160 }, { 252, 158 }, { 252, 158 }, { 252, 158 }, { 252, 158 },
	{ 252, 158 }, { 256, 160 }, { 256, 160 }, { 252, 160 }, { 232, 163 },
	{ 232, 169 }, { 218, 169 }, { 206, 172 }, { 201, 184 }, { 198, 212 },
	{ 194, 231 }, { 194, 231 }, { 194, 217 }, { 200, 201 }, { 228, 183 },
	{ 244, 181 }, { 258, 176 }, { 276, 173 }, { 295, 141 }, { 309, 136 },
	{ 328, 113 }, { 346, 103 }, { 353, 80 }, { 354, 58 }, { 364, 53 }
};

static const char *SAID[][4] = {
	{ "FRONT DOOR",      "502w003", "500w001", "502w057" },
	{ "KITCHEN",         nullptr,   "500w001", nullptr   },
	{ "STAIRS",          "502w007", "500w001", nullptr   },
	{ "BORK",            "502w009", "500w002", "500w002" },
	{ "BORK ",           "502w011", "500w002", "500w002" },
	{ "CHARRED OUTLINE", "502w012", "502w013", "500w001" },
	{ "RAILING",         "502w014", nullptr,   "502w016" },
	{ "KINDLING ",       "502w018", nullptr,   "502w021" },
	{ "FIREPLACE",       "502w023", "500w001", "502w022" },
	{ "WINDOW",          "500w003", "500w001", "500w004" },
	{ "CHANDELIER",      "502w027", "502w028", "502w028" },
	{ "WIRES",           "502w029", "502w030", "502w030" },
	{ "WIRES ",          "502w031", "502w030", "502w030" },
	{ "PHONE JACK",      "502w032", nullptr,   "502w032" },
	{ "ARMCHAIR",        "502w034", nullptr,   "502w035" },
	{ "ROCKER",          "502w036", nullptr,   "502w035" },
	{ "LAMP",            "502w038", "502w039", "502w039" },
	{ "PIANO",           "502w040", "502w041", "502w042" },
	{ "SHEET MUSIC",     "502w043", "502w044", "502w045" },
	{ "BOOKS",           "502w046", "502w047", "502w047" },
	{ "BOOK",            "502w048", "502w049", "502w047" },
	{ "PICTURE",         "502w050", "500w005", "502w051" },
	{ "PICTURE ",        "502w052", "500w005", "502w051" },
	{ "PAINTING",        "502w053", "502w054", "502w051" },
	{ nullptr, nullptr, nullptr, nullptr }
};


void Room502::init() {
	_G(flags)[V194] = 0;
	Section5Room::init();

	pal_cycle_init(124, 127, 12);
	loadSeries1();
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;

	bool skip = false;
	switch (_G(game).previous_room) {
	case 503:
		ws_demand_location(620, 311, 9);
		ws_walk(304, 308, 0, -1, -1);
		player_set_commands_allowed(true);
		kernel_trigger_dispatch_now(24);
		break;

	case 505:
		ws_demand_location(237, 235, 9);
		ws_hide_walker();
		_val1 = _G(flags)[V196] ? 6 : 4;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	case 506:
		ws_demand_location(402, 272, 1);
		_val1 = 12;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_val1 = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(304, 308, 9);
		ws_hide_walker();
		kernel_trigger_dispatch_now(5);
		skip = true;
		break;
	}

	if (!skip) {
		setup1();
		setup2();
		setup3();
	}

	series_show("502logs", 0xc00);

	if (_G(flags)[V198])
		kernel_trigger_dispatch_now(19);

	_flag1 = true;
	Section5Room::init();
}

void Room502::daemon() {
}

void Room502::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("KITCHEN") && player_said_any("LOOK AT", "GEAR"))
		player_set_facing_hotspot();
}

void Room502::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool railing = player_said("RAILING") && _G(flags)[V200] == 5003;
	bool takeKindling = player_said("KINDLING ") && player_said("TAKE");
	bool gearKindling = player_said("KINDLING ") && player_said("GEAR");
	bool fireplace = player_said("FIREPLACE") && _G(flags)[V198] != 0;

	if (player_said("LOOK AT", "FRONT DOOR") && _G(flags)[V195]) {
		wilbur_speech("502w004");
	} else if (player_said("LOOK AT STAIRS") && _G(flags)[V200] == 5003) {
		wilbur_speech("502w008");
	} else if (railing && player_said("LOOK AT")) {
		wilbur_speech("502w015");
	} else if (railing && player_said("GEAR")) {
		wilbur_speech("502w017");
	} else if (takeKindling && inv_player_has("kindling")) {
		wilbur_speech("502w019");
	} else if (takeKindling && _G(flags)[V198]) {
		wilbur_speech("502w020");
	} else if (gearKindling && inv_player_has("KINDLING")) {
		wilbur_speech("502w022");
	} else if (gearKindling && _G(flags)[V198]) {
		wilbur_speech("502w020");
	} else if (player_said("KINDLING") && player_said("FIREPLACE")) {
		wilbur_speech(_G(flags)[V198] ? "500w063" : "500w062");
	} else if (player_said("KINDLING") && player_said("WIRES")) {
		wilbur_speech("500w065");
	} else if (fireplace && player_said("LOOK AT")) {
		wilbur_speech("502w024");
	} else if (fireplace && player_said("GEAR")) {
		wilbur_speech("502w025");
	} else if (fireplace && player_said("RUBBER DUCK")) {
		wilbur_speech("502w030");
	} else if (player_said("GEAR", "WINDOW") && _G(flags)[V200] == 5003) {
		wilbur_speech("502w026");
	} else if ((player_said("LOOK AT") || player_said("GEAR")) &&
			player_said("PHONE JACK") && _G(flags)[V197] != 0) {
		wilbur_speech("502w033");
	} else if (player_said("LOOK AT", "ROCKER") && _G(flags)[V197] != 0) {
		wilbur_speech("502w037");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "FRONT DOOR")) {
		_val1 = 2;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("KITCHEN") && player_said_any("LOOK AT", "GEAR")) {
		_val6 = 5007;
		kernel_trigger_dispatch_now(1);
	} else if (player_said("GEAR", "STAIRS")) {
		_val1 = 3;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BORK ") && player_said("LOOK AT") &&
			!player_said_any("GIZMO", "ROLLING PIN", "DIRTY SOCK", "SOAPY WATER", "RUBBER GLOVES") &&
			!player_said("LAXATIVE")) {
		player_set_commands_allowed(false);
		_val3 = 26;
		++_val2;
	} else if (player_said("TAKE", "KINDLING ")) {
		if (!_G(flags)[V198] && !inv_player_has("KINDLING")) {
			_val1 = 10;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("SOAPY WATER", "RAILING")) {
		_val1 = 5;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BURNING KINDLING", "FIREPLACE")) {
		_val1 = 12;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		inv_move_object("BURNING KINDLING", NOWHERE);
	} else {
		return;
	}

	_G(player).command_ready = false;
}
void Room502::loadSeries1() {
	static const char *NAMES[6] = {
		"500_008", "500_009", "500_010", "500_011", "500_012", "500_013"
	};
	for (int i = 0; i < 6; ++i)
		digi_preload(NAMES[i]);
	series_load("502spark");
}

void Room502::loadSeries2() {
	static const char *NAMES[15] = {
		"502bk01", "502bk01s", "502bk02", "502bk02s", "502bk03",
		"502bk03s", "502bk04", "502bk04s", "502bk05", "502bk06",
		"502bk06s", "502bk07", "502bk07s", "502bk08", "502bk08s"
	};
	for (int i = 0; i < 15; ++i)
		series_load(NAMES[i]);
}

void Room502::loadSeries3() {
	series_load("502bk09");
	series_load("502bk09s");
	series_load("502bkst");
	series_load("502bksts");
	digi_preload("502b001a");
	digi_preload("502b001b");
	digi_preload("502b003a");
	digi_preload("502b003b");
	digi_preload("502b004");
}

void Room502::setup1() {
	if (_G(flags)[V197]) {
		series_show("502bk05", 0xd00);
		series_show("502phone", 0x800);
		hotspot_set_active("BORK ", false);
		hotspot_set_active("CHARRED OUTLINE", true);

	} else {
		loadSeries2();
		hotspot_set_active("BORK ", true);
		hotspot_set_active("CHARRED OUTLINE", false);

		_walk1 = intr_add_no_walk_rect(370, 281, 500, 310, 328, 318);

		if (_G(game).previous_room == 503) {
			series_show("502bk01", 0x400);
			_series1.show("502bk01", 0x400);
			_flag1 = true;
			_val3 = 25;
			kernel_timing_trigger(120, 10);

		} else {
			_val3 = 20;
			kernel_trigger_dispatch_now(10);
		}
	}
}

void Room502::setup2() {
	if (_G(flags)[V200] == 5003) {
		_val5 = 36;
		kernel_trigger_dispatch_now(17);
		kernel_trigger_dispatch_now(18);
	} else {
		loadSeries3();
		_val4 = _G(game).previous_room == 505 ? 33 : 27;
		kernel_trigger_dispatch_now(13);
	}
}

void Room502::setup3() {
	if (_G(flags)[V211] == 5000) {
		_series2 = series_show("502spark", 0xc00);
		kernel_trigger_dispatch_now(21);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
