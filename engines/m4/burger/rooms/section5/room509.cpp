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

#include "m4/burger/rooms/section5/room509.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room509::SAID[][4] = {
	{ "HALLWAY",            nullptr, "500w001",   nullptr   },
	{ "CHRISTMAS LIGHTS  ", "509w002", nullptr,   nullptr   },
	{ "PHONE CORD",         "509w005", "509w006", "509w007" },
	{ "HOLE",               "509w008", "500w001", "509w009" },
	{ "ROOF BORK",          nullptr,   nullptr,   nullptr   },
	{ "BED",                "509w010", nullptr,   "509w011" },
	{ "MATTRESS",           "509w010", nullptr,   "509w011" },
	{ "PILLOW",             "509w012", "509w013", "509w013" },
	{ "LAMP",               "509w014", "509w015", "509w014" },
	{ "WINDOW",             "500w003", nullptr,   "500w004" },
	{ "CHAIR",              "509w016", nullptr,   "509w017" },
	{ "TELEPHONE",          "509w018", "509w019", "509w020" },
	{ "BOOK",               "509w021", "509w022", "509w023" },
	{ "PICTURE",            "509w024", "500w005", nullptr   },
	{ "PICTURE ",           "509w025", "500w005", "509w026" },
	{ "PICTURE  ",          "509w027", "500w005", "509w026" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room509::PLAY1[] = {
	{  0, 14, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 18, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY3[] = {
	{  0, -1, nullptr,   0,   0, -1, 0,  0, nullptr,  0 },
	{  0, 15, nullptr,   0,   0, -1, 0,  0, nullptr,  0 },
	{ 16, -1, nullptr,   0,   0,  3, 0,  0, nullptr,  0 },
	{ 16, -1, "500w086", 1, 255, -1, 4, -1, &_state1, 1 },
	{ 16, -1, nullptr,   0,   0, -1, 4,  2, &_state1, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY4[] = {
	{  0, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  0, 15, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 16, -1, "509_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY7[] = {
	{ 0, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, "509_002", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY8[] = {
	{ 0,  4, "508b003", 2, 255, -1, 0, 0, nullptr,  0 },
	{ 5, -1, "500_005", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 5, -1, "500_006", 2, 255, -1, 0, 0, &_state2, 2 },
	PLAY_BREAK_END
};

long Room509::_state1;
long Room509::_state2;


Room509::Room509() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room509::init() {
	Section5Room::init();
	pal_cycle_init(109, 124, 6);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		if (inv_player_has("CHRISTMAS LIGHTS") || inv_player_has("CHRISTMAS LIGHTS ")) {
			disable_player();
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 505:
		ws_demand_location(4, 286, 3);
		ws_unhide_walker();

		if (player_been_here(509)) {
			ws_walk(272, 325, nullptr, -1);
		} else {
			ws_walk(272, 325, nullptr, 1);
		}
		break;

	default:
		ws_demand_location(272, 325);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;
	}

	_state1 = 0;

	if (_G(flags)[V227]) {
		_val1 = 17;
	} else {
		digi_preload("508b001a");
		digi_preload("508b001b");
		digi_preload("508b001c");
		digi_preload("508b001d");
		_val1 = 15;
	}

	kernel_trigger_dispatch_now(2);

	if (_G(flags)[V229] == 0 && !inv_player_has("CHRISTMAS LIGHTS") &&
			!inv_player_has("CHRISTMAS LIGHTS ")) {
		hotspot_set_active("CHRISTMAS LIGHTS  ", true);
		series_load("509wi05");
		series_load("509wi06");
		series_load("509wi07");
		series_load("509wi08");

		series_show((_G(flags)[V212] == 5000) ? "509lgt02" : "509lgt01", 0xc00);			
	} else {
		hotspot_set_active("CHRISTMAS LIGHTS  ", false);
	}

	_series2 = series_show("509wire", 0x900);

	_initFlag = true;
	Section5Room::init();
}

void Room509::daemon() {
	// TODO
}

void Room509::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!inv_player_has("CHRISTMAS LIGHTS") && !inv_player_has("CHRISTMAS LIGHTS ")) {
		if (player_said("HALLWAY") && !player_said_any("LEAVE", "LOOK AT", "GEAR"))
			player_set_facing_hotspot();
	} else {
		_G(wilbur_should) = 12;

		if (player_said("CHRISTMAS LIGHTS ", "PHONE CORD")) {
			_val2 = _G(flags)[V234] ? 6 : 5;
		} else if (player_said("CHRISTMAS LIGHTS", "PHONE CORD")) {
			_G(wilbur_should) = 5;
		} else if (player_said("LOOK AT") && player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ")) {
			_val2 = player_said("CHRISTMAS LIGHTS") ? 1 : 0;
		} else if (player_said("HOLE") && player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ")) {
			_val2 = 2;
		} else if (player_said("CHRISTMAS LIGHTS") || player_said("CHRISTMAS LIGHTS ")) {
			_val2 = 4;
			intr_cancel_sentence();
			_G(player).need_to_walk = false;
		} else {
			_G(wilbur_should) = 3;
		}

		_G(player).ready_to_walk = false;
		terminateMachineAndNull(_general1);
		terminateMachineAndNull(_general2);
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	}
}

void Room509::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT", "CHRISTMAS LIGHTS  ") && _G(flags)[V212] == 5001) {
		wilbur_speech("509w003");
	} else if (player_said("GEAR", "CHRISTMAS LIGHTS  ")) {
		wilbur_speech("509w004");
	} else if (player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ") && player_said("HOLE")) {
		_val2 = 2;
		kernel_trigger_dispatch_now(12);
	} else if (player_said("LOOK AT", "HOLE") && _G(flags)[V227] != 0) {
		wilbur_speech("500w003");
	} else if (player_said("TAKE", "TELEPHONE") && _G(flags)[V197]) {
		wilbur_speech("500w005");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("HALLWAY") && player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 5009);
	} else if (player_said("TAKE", "CHRISTMAS LIGHTS  ")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
