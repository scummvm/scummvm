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

#include "m4/burger/rooms/section5/room507.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room507::SAID[][4] = {
	{ "HALLWAY",            nullptr,   "500w001", nullptr   },
	{ "BORK",               "507w001", "500w002", "500w002" },
	{ "RUBBER DUCKY ",      "507w004", nullptr,   "507w006" },
	{ "RUBBER DUCKY  ",     "507w004", "507w005", "507w006" },
	{ "TOILET",             "507w007", "500w001", nullptr   },
	{ "FLUSH CHAIN",        "507w007", "500w001", nullptr   },
	{ "TUB",                "507w001", "500w001", "500w002" },
	{ "WINDOW",             "500w003", "500w001", "500w004" },
	{ "TOILET SEAT",        "507w012", "500w005", "500w005" },
	{ "SHOWER CURTAIN",     nullptr,   "500w005", nullptr   },
	{ "SHOWERHEAD",         "507w013", "507w014", nullptr   },
	{ "CLOSET",             "507w015", "500w001", "500w001" },
	{ "TOILET PAPER",       "507w016", "500w005", "500w005" },
	{ "TOWELS",             "507w017", "507w018", "507w018" },
	{ "DRAIN CLEANER",      "507w019", "507w020", "507w020" },
	{ "MIRROR",             "507w021", nullptr,   "507w022" },
	{ "SINK",               "507w023", nullptr,   nullptr   },
	{ "BOARD",              "507w024", "500w005", nullptr   },
	{ "LIVER SPOT CREAM",   "507w025", "507w026", "507w026" },
	{ "FACIAL HAIR BLEACH", "507w027", "507w028", "507w029" },
	{ "WRINKLE CREAM",      "507w030", "507w031", "507w031" },
	{ "FACE CREAM",         "507w032", "507w020", "507w033" },
	{ "WIRES",              "507w034", "507w035", "507w035" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room507::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY2[] = {
	{ 0,  4, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 5, -1, nullptr, 0, 0, 11, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY3[] = {
	{ 0, 5, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 5, "500_004", 1, 255, -1, 0, 3, nullptr, 0 },
	{ 0, 3, nullptr,   0,   0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY4[] = {
	{ 11, 16, nullptr,   0,   0,    -1, 0, 0, nullptr, 0 },
	{ 17, 19, "507_006", 1, 255,    11, 0, 0, nullptr, 0 },
	{ 20, 23, nullptr,   0,   0, 10016, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY5[] = {
	{ 0, 10, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY6[] = {
	{ 0, 10, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY7[] = {
	{ 0, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0,  9, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY8[] = {
	{  0, 12, nullptr,   0,   0, -1,    0, 0, nullptr,  0 },
	{ 13, 14, "507_004", 1, 255, -1,    0, 0, nullptr,  0 },
	{ 15, 15, nullptr,   0,   0, -1,    0, 8, nullptr,  0 },
	{ 16, 27, nullptr,   1,   0, -1, 2048, 0, nullptr,  0 },
	{ 28, -1, nullptr,   0,   0, 11,    0, 0, nullptr, -1 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY9[] = {
	{  0,  6, "507b001a", 3, 255, -1, 0, 0, &_state1, 1 },
	{  0,  6, "507b001b", 3, 255, -1, 0, 0, &_state1, 2 },
	{  0,  6, "507b001c", 3, 255, -1, 0, 0, &_state1, 3 },
	{  0,  6, "507b001d", 3, 255, -1, 0, 0, &_state1, 4 },
	{  7, 11, "507_002a", 2, 255, -1, 0, 0, &_state2, 1 },
	{  7, 11, "507_002b", 2, 255, -1, 0, 0, &_state2, 2 },
	{  7, 11, "507_002c", 2, 255, -1, 0, 0, &_state2, 3 },
	{  7, 11, "507_002d", 2, 255, -1, 0, 0, &_state2, 4 },
	{  7, 11, "507_002e", 2, 255, -1, 0, 0, &_state2, 5 },
	{  7, 11, "507_002f", 2, 255, -1, 0, 0, &_state2, 6 },
	{ 12, 21, "507_002a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 12, 21, "507_002b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 12, 21, "507_002c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 12, 21, "507_002d", 2, 255, -1, 0, 0, &_state3, 4 },
	{ 12, 21, "507_002e", 2, 255, -1, 0, 0, &_state3, 5 },
	{ 12, 21, "507_002f", 2, 255, -1, 0, 0, &_state3, 6 },
	{ 22, -1, "507_002a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 22, -1, "507_002b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 22, -1, "507_002c", 2, 255, -1, 0, 0, &_state4, 3 },
	{ 22, -1, "507_002d", 2, 255, -1, 0, 0, &_state4, 4 },
	{ 22, -1, "507_002e", 2, 255, -1, 0, 0, &_state4, 5 },
	{ 22, -1, "507_002f", 2, 255, -1, 0, 0, &_state4, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY10[] = {
	{ 0, -1, "507_001", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY11[] = {
	{  0,  1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  2,  9, "507b003a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  2,  9, "507b003b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  2,  9, "507b003c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 10, 15, "507b004a", 2, 255, -1, 4, 2, &_state2, 1 },
	{ 10, 15, "507b004b", 2, 255, -1, 4, 2, &_state2, 2 },
	{ 10, 15, "507b004c", 2, 255, -1, 4, 2, &_state2, 3 },
	{ 16, 20, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 21, 21, nullptr,    0,   0, -1, 0, 8, nullptr,  0 },
	{ 22, -1, "507b005",  2, 255, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY12[] = {
	{  0, 11, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 12, 15, "507b006a", 2, 255, -1, 4, 2, &_state1, 1 },
	{ 12, 15, "507b006b", 2, 255, -1, 4, 2, &_state1, 2 },
	{ 16, 16, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 17, 18, "500_004",  2, 255, -1, 0, 3, nullptr,  0 },
	{ 19, 28, "507b007a", 2, 255, -1, 4, 2, &_state2, 1 },
	{ 19, 28, "507b007b", 2, 255, -1, 4, 2, &_state2, 2 },
	{ 29, 32, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 33, -1, "507b008a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 33, -1, "507b008b", 2, 255, -1, 0, 0, &_state3, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY13[] = {
	{  0,  3, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  4,  8, "507b003a", 2, 255, -1, 4, 0, &_state1, 1 },
	{  4,  8, "507b003b", 2, 255, -1, 4, 0, &_state1, 2 },
	{  4,  8, "507b003c", 2, 255, -1, 4, 0, &_state1, 3 },
	{ 20, 37, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 38, 46, "507b010a", 2, 255, -1, 1, 0, &_state2, 1 },
	{ 38, 46, "507b010b", 2, 255, -1, 1, 0, &_state2, 2 },
	{ 47, 59, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 60, 78, "500_014",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 79, -1, "507_007",  2, 255, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY14[] = {
	{  0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  8, 13, "507b011", 2, 255, -1, 4, 0, nullptr, 0 },
	{ 14, 29, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 30, -1, "507_005", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY15[] = {
	{ 0, -1, "507b013", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY16[] = {
	{ 0,  6, "507b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 0,  6, "507b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 0,  6, "507b002c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 7,  7, nullptr,    0,   0,  8, 0, 0, &_state5, 3 },
	{ 7, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY17[] = {
	{ 0,  3, "507_003", 1, 255, -1, 1, 5, nullptr, 0 },
	{ 4, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY18[] = {
	{ 6, 6, nullptr, 0, 0, -1, 0, 5, nullptr, 0 },
	{ 5, 5, nullptr, 0, 0, -1, 0, 5, nullptr, 0 },
	{ 4, 4, nullptr, 0, 0, -1, 0, 5, nullptr, 0 },
	PLAY_BREAK_END
};

long Room507::_state1;
long Room507::_state2;
long Room507::_state3;
long Room507::_state4;
long Room507::_state5;


Room507::Room507() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
	_state5 = 0;
}

void Room507::init() {
	Section5Room::init();
	pal_cycle_init(112, 127, 6);

	for (_ctr = 0; _ctr < 5; ++_ctr)
		_triggers[_ctr] = -1;

	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;
	_flag1 = _flag2 = false;

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 505:
		ws_demand_location(610, 280, 1);

		if (player_been_here(507)) {
			ws_walk(343, 323, nullptr, -1);
		} else {
			_val1 = 12;
			ws_walk(343, 323, nullptr, 3);
		}
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(343, 323);
		break;
	}

	_state5 = 0;
	hotspot_set_active("BORK", false);

	if (_G(flags)[V223] != 2) {
		loadSeries();

		if (_G(flags)[V223] == 1) {
			hotspot_set_active_xy("BORK", 123, 218, true);
			_val2 = 22;
		} else {
			hotspot_set_active_xy("BORK", 320, 220, true);
			_val2 = 15;
			kernel_trigger_dispatch_now(5);
		}

		kernel_trigger_dispatch_now(6);
	}

	switch (inv_where_is("RUBBER DUCKY")) {
	case 500:
		hotspot_set_active("RUBBER DUCKY ", false);
		hotspot_set_active("RUBBER DUCKY  ", true);
		break;

	case  507:
		_series1 = series_show("507duck", 0xa00);
		hotspot_set_active("RUBBER DUCKY ", true);
		hotspot_set_active("RUBBER DUCKY  ", false);
		break;

	default:
		hotspot_set_active("RUBBER DUCKY ", false);
		hotspot_set_active("RUBBER DUCKY  ", false);
		break;
	}

	if (_G(flags)[V223] != 1) {
		_val3 = inv_where_is("RUBBER DUCKY") == 500 ? 27 : 26;
		kernel_trigger_dispatch_now(9);
	}

	kernel_trigger_dispatch_now(10);
	series_show("507tub", 0xf00);

	if (_G(flags)[V228])
		series_show("507windo", 0xf00);

	_initFlag = true;
	Section5Room::init();
}

void Room507::daemon() {
}

void Room507::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("HALLWAY") && player_said_any("GEAR", "LOOK AT", "ENTER"))
		player_set_facing_hotspot();
}

void Room507::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool tubFlag = player_said("TUB") && _G(flags)[V223] != 0;

	if (player_said("LOOK AT", "BORK") && _G(flags)[V223] == 1) {
		wilbur_speech("507w003");
	} else if (player_said("RUBBER DUCKY") && player_said("TUB")) {
		wilbur_speech(_G(flags)[223] ? "500w040" : "500w039");
	} else if (player_said("RUBBER DUCKY") && player_said("SINK")) {
		wilbur_speech("500w041");
	} else if (player_said("SOAPY WATER", "SINK")) {
		wilbur_speech("500w057");
	} else if (player_said("LOOK AT") && player_said_any("TOILET", "FLUSH CHAIN") &&
			_G(flags)[V223] == 1) {
		wilbur_speech("507w008");
	} else if (tubFlag && player_said("LOOK AT")) {
		wilbur_speech("507w009");
	} else if (tubFlag && player_said("GEAR")) {
		wilbur_speech("507w010");
	} else if (player_said("GEAR", "WINDOW") && _G(flags)[V228]) {
		wilbur_speech("507w011");
	} else if (player_said("GEAR") && (player_said("SHOWER CURTAIN") || player_said("SHOWERHEAD")) &&
			_G(flags)[V223] == 2) {
		// No implementation
	} else if (_G(walker).wilbur_said(SAID)) {
		// No implementation
	} else if (player_said("LOOK AT", "HALLWAY") || player_said("GEAR", "HALLWAY")) {
		_val4 = 5009;
		kernel_trigger_dispatch_now(2);
	} else if (_G(flags)[V223] != 1 && player_said("RUBBER DUCKY", "BORK")) {
		player_set_commands_allowed(false);
		_val5 = 6;
		_val6 = 19;
		kernel_trigger_dispatch_now(4);
	} else if (_G(flags)[V223] != 1 && player_said("BORK") && !player_said("LOOK AT") &&
			!player_said_any("GIZMO", "ROLLING PIN", "DIRTY SOCK", "SOAPY WATER", "RUBBER GLOVES") &&
			!player_said("LAXATIVE")) {
		player_set_commands_allowed(false);
		_val2 = 23;
	} else if (player_said("TAKE", "RUBBER DUCKY ")) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("RUBBER DUCKY") && player_said_any("GEAR", "WILBUR")) {
		_G(wilbur_should) = 3;
		ws_walk(197, 274, nullptr, gCHANGE_WILBUR_ANIMATION, 9);
	} else if (player_said("RUBBER DUCKY", "TOILET")) {
		_flag2 = true;
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("GEAR") && player_said_any("TOILET", "FLUSH CHAIN")) {
		if (_G(flags)[V223] == 1 || _G(flags)[V223] == 2) {
			_G(wilbur_should) = 9;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		} else if (inv_where_is("RUBBER DUCKY") == 500) {
			_val5 = 9;
			_val6 = 21;
			kernel_trigger_dispatch_now(4);
		} else {
			_val5 = 9;
			_val6 = 20;
			kernel_trigger_dispatch_now(4);
		}
	} else if (player_said("BOTTLE") && player_said_any("SOAPY WATER ", "TUB ")) {
		if (_G(flags)[V223] == 1 || _G(flags)[V223] == 2) {
			_G(wilbur_should) = 11;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		} else {
			wilbur_speech("507w038");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room507::loadSeries() {
	digi_preload("507b012a");
	digi_preload("507b012b");
	digi_preload("507b012c");
	digi_preload("507b012d");
	digi_preload("507b012e");

	if (_G(flags)[V223] != 1) {
		series_load("507bk01");
		series_load("507bk03");
		series_load("507bk04");
		series_load("507bk08");
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
