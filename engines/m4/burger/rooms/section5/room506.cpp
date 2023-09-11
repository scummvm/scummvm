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

#include "m4/burger/rooms/section5/room506.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room506::SERIES1[] = {
	{   0, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  13, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  20, "506_006",  1, 255, -1, 0, nullptr,  0 },
	{  45, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  67, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  77, "506_003",  2, 255, -1, 0, nullptr,  0 },
	{  91, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 100, "506b005a", 1, 255, -1, 0, &_state1, 1 },
	{ 100, "506b005b", 1, 255, -1, 0, &_state1, 2 },
	{ 100, "506b005c", 1, 255, -1, 0, &_state1, 3 },
	{ 113, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 135, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 155, "506_001",  2, 255, -1, 0, nullptr,  0 },
	{ 165, nullptr,    0,   0, 11, 0, nullptr,  0 },
	STREAM_BREAK_END
};

const char *Room506::SAID[][4] = {
	{ "HALLWAY",        nullptr,   "500w001", nullptr   },
	{ "WINDOW",         "500w003", "500w001", nullptr   },
	{ "ROOF",           "500w003", "500w001", nullptr   },
	{ "BORK",           "506w004", "500w002", "500w002" },
	{ "TELEVISION",     "506w005", nullptr,   "506w007" },
	{ "FIRE",           "506w009", "506w010", "506w011" },
	{ "VIDEO GAME",     "506w012", "506w013", "506w015" },
	{ "BED",            "506w017", nullptr,   "506w018" },
	{ "MATTRESS",       "506w017", nullptr,   "506w018" },
	{ "READING LAMP",   "506w019", "506w020", "506w021" },
	{ "NIGHT TABLE",    "506w022", "500w005", "500w005" },
	{ "PILLOW",         "506w023", "506w024", "506w025" },
	{ "LAMP",           "506w026", "506w020", "506w021" },
	{ "BOOK",           "506w027", "506w028", "506w028" },
	{ "WARDROBE",       "506w029", nullptr,   nullptr   },
	{ "CLOTHES",        "506w030", "506w031", "506w032" },
	{ "DRESSER",        "506w033", nullptr,   nullptr   },
	{ "MODEL ROCKET ",  "506w034", "500w005", "500w005" },
	{ "MODEL ROCKET",   "506w035", "506w036", "500w005" },
	{ "MODEL ROCKET  ", "506w037", "500w005", "500w005" },
	{ "POSTER ",        "506w038", "500w005", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room506::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY3[] = {
	{  0, 25, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 26, 40, nullptr,   0,   0,  5, 0,  0, nullptr, 0 },
	{ 41, 46, "500w067", 1, 255, -1, 4, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY4[] = {
	{  9, 22, "506_003",  2,  50, -1, 0, 0, nullptr,  0 },
	{ 23, 28, "506b003a", 2, 255, -1, 4, 2, &_state1, 1 },
	{ 23, 28, "506b003b", 2, 255, -1, 4, 2, &_state1, 2 },
	{ 29, 37, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY5[] = {
	{ 38, 38, "506_003",  2, 120, -1, 0, 12, nullptr,  0 },
	{ 39, 73, "506b004a", 2, 255, -1, 0,  0, &_state1, 1 },
	{ 39, 73, "506b004b", 2, 255, -1, 0,  0, &_state1, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY6[] = {
	{  0, 12, "506b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0, 12, "506b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 13, 13, nullptr,    0,   0,  9, 0, 0, &_state2, 3 },
	{ 13, 30, nullptr,    0,   0,  6, 0, 0, nullptr,  0 },
	{ 31, -1, nullptr,    0,   0,  2, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};


long Room506::_state1;
long Room506::_state2;

Room506::Room506() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room506::init() {
	Section5Room::init();
	pal_cycle_init(112, 127, 6);

	for (_ctr = 0; _ctr < 5; ++_ctr)
		_array1[_ctr] = -1;

	if (_G(flags)[V219]) {
		kernel_trigger_dispatch_now(12);
	} else {
		hotspot_set_active("FIRE", false);
	}

	if (_G(flags)[V219] == 0) {
		_val1 = 19;
		kernel_trigger_dispatch_now(10);
	}

	if (_G(flags)[V219]) {
		series_show("506windo", 0xf00);
	} else {
		hotspot_set_active("ROOF", false);
	}

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 505:
		if (_G(flags)[V218] != 5003)
			_G(flags)[V218] = 5000;

		player_set_commands_allowed(true);
		ws_demand_location(543, 252, 9);

		if (player_been_here(506)) {
			ws_walk(434, 254, nullptr, -1, 9);
		} else {
			ws_walk(434, 254, nullptr, 6, 9);
		}
		break;

	case 508:
		if (_G(flags)[V218] != 5003)
			_G(flags)[V218] = 5000;

		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(434, 254, 9);
		break;
	}

	_state2 = 0;

	if (_G(flags)[V218] == 5003) {
		hotspot_set_active("BORK", false);
	} else {
		loadSeries();
		_val2 = 12;
		kernel_trigger_dispatch_now(7);
		_walk1 = intr_add_no_walk_rect(308, 278, 430, 314, 303, 328);
	}

	_initFlag = true;
	Section5Room::init();
}

void Room506::daemon() {
	// TODO
}

void Room506::pre_parser() {
	if (player_said("AMPLIFIER", "TELEVISION") && _G(flags)[V218] != 5003) {
		_G(wilbur_should) = 4;
		player_hotspot_walk_override(456, 245, 8, gCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	} else if (player_said("FIRE", "KINDLING")) {
		_G(wilbur_should) = 7;
		player_hotspot_walk_override(409, 311, 4, gCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	} else if (player_said("HALLWAY") && player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		player_set_facing_hotspot();
	}
}

void Room506::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("GEAR") && player_said_any("WINDOW", "ROOF") && !_G(flags)[V219]) {
		wilbur_speech("500w004");
	} else if (player_said("TELEVISION") && player_said("LOOK AT")) {
		wilbur_speech("506w006");
	} else if (player_said("TELEVISION") && player_said("GEAR")) {
		wilbur_speech("506w008");
	} else if (_G(flags)[V219] != 0 && player_said("VIDEO GAME") &&
			player_said_any("TAKE", "GEAR")) {
		// No implementation
	} else if (_G(walker).wilbur_said(SAID)) {
		// No implementation
	} else if (player_said("BORK") && player_said("LOOK AT") &&
			!player_said_any("GIZMO", "ROLLING PIN", "DIRTY SOCK", "SOAPY WATER", "RUBBER GLOVES") &&
			!player_said("LAXATIVE")) {
		player_set_commands_allowed(false);
		_val2 = 15;
		++_state2;
	} else if (player_said_any("ROOF", "WINDOW") && !player_said_any("ENTER", "GEAR") &&
			_G(flags)[V219]) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("HALLWAY") && player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		_val3 = 5009;
		kernel_trigger_dispatch_now(3);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room506::loadSeries() {
	series_load("506bk01");
	series_load("506bk01s");
	series_load("506bk02");
	series_load("506bk02s");
	series_load("506bk03");
	series_load("506bk03s");
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
