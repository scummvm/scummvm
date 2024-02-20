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

#include "m4/burger/rooms/section6/room604.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_GERBILS_ANIMATION = 6011,
	kCHANGE_WOODCHIPS_ANIMATION = 6012
};

static const char *SAID[][4] = {
	{ "WOOD SHAVINGS", nullptr,   "604w010", nullptr   },
	{ "ASHES",         "604w015", "604w016", "604w016" },
	{ "APPLE CORE",    "604w017", "604w018", "604w019" },
	{ "BOTTLE CAP",    "604w020", "604w021", "604w010" },
	{ "TUBE",          "604w022", "604w023", nullptr   },
	{ "TUBE ",         "604w024", "604w023", nullptr   },
	{ "BARS",          "604w025", "604w023", "604w023" },
	{ "FLOOR",         "604w026", "604w023", "604w023" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesStreamBreak SERIES1[] = {
	{   4, "604I001",  1, 255, -1, 0, nullptr, 0 },
	{  29, "604Z001",  1, 255, -1, 0, nullptr, 0 },
	{  36, "604I002",  1, 255, -1, 0, nullptr, 0 },
	{  45, "604Z002",  1, 255, -1, 0, nullptr, 0 },
	{  70, "604Z003",  1, 255, -1, 0, nullptr, 0 },
	{ 106, "604I003",  1, 255, -1, 0, nullptr, 0 },
	{ 132, "604Z004A", 1, 255, -1, 0, nullptr, 0 },
	{ 208, "604Z004B", 1, 255, -1, 0, nullptr, 0 },
	{ 245, "604Z004C", 1, 255, -1, 0, nullptr, 0 },
	{ 324, "604I004",  1, 255, -1, 0, nullptr, 0 },
	{ 344, "604Z005",  1, 255, -1, 0, nullptr, 0 },
	{ 374, "604Z006",  1, 255, -1, 0, nullptr, 0 },
	{ 409, "604I005",  1, 255, -1, 0, nullptr, 0 },
	{ 459, "604Z007",  1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room604::PLAY1[] = {
	{ 0, -1, "604_007", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY2[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w016", 1, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY3[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w015", 1, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY4[] = {
	{  0, 19, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 20, 21, "600_015", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 22, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY5[] = {
	{  0, 22, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 23, -1, "600_015", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY6[] = {
	{ 0,  5, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 6, -1, "604_001", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY7[] = {
	{ 0,  5, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 6, -1, "604_001", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY8[] = {
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 0 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 1 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 2 },
	{ 7, -1, "600w011a", 1, 255, -1,    0, 0, &_state1, 0 },
	{ 7, -1, "600w011b", 1, 255, -1,    0, 0, &_state1, 1 },
	{ 7, -1, "600w011c", 1, 255, -1,    0, 0, &_state1, 2 },
	{ 0, -1, "600w011d", 1, 255, -1,    0, 0, &_state1, 3 },
	{ 0, -1, "600w011e", 1, 255, -1,    0, 0, &_state1, 4 },
	{ 0, -1, "600w011f", 1, 255, -1,    0, 0, &_state1, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY9[] = {
	{  0, 13, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 14, -1, "600_012", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY10Demo[] = {
	{  0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{  4, 14, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 15, 23, nullptr,   1, 255, -1,    0, 0, nullptr, 0 },
	{ 24, -1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY10[] = {
	{  0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{  4, 14, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 15, 23, "604_008", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 24, -1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY11[] = {
	{  0,  7, nullptr, 1, 0,    -1, 2048, 0, nullptr, 0 },
	{  8, 17, nullptr, 1, 0,    -1,    0, 0, nullptr, 0 },
	{ 18, -1, nullptr, 1, 0, 10016,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY12[] = {
	{ 0, 15,  nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 16, -1, "600_014", 2, 255, -1,    0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

int32 Room604::_state1;

Room604::Room604() : Section6Room() {
	_state1 = 0;
}

void Room604::init() {
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;

	if (_G(flags)[V274]) {
		hotspot_set_active("WOOD SHAVINGS", false);
		hotspot_set_active("ASHES", true);
		_woodchipsShould = 27;
		kernel_trigger_dispatch_now(kCHANGE_WOODCHIPS_ANIMATION);

	} else {
		hotspot_set_active("WOOD SHAVINGS", true);
		hotspot_set_active("ASHES", false);

		if (_G(game).previous_room != 601) {
			_woodchipsShould = 25;
			kernel_trigger_dispatch_now(kCHANGE_WOODCHIPS_ANIMATION);
		}
	}

	_G(flags)[kStandingOnKibble] = 0;

	if (_G(flags)[V273] == 1) {
		series_show("602spill", 0x900);
		_G(kernel).call_daemon_every_loop = true;
	}

	if (_G(flags)[V245] == 10030) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	if (_G(flags)[kHampsterState] == 6006) {
		_G(wilbur_should) = 18;
		Section6::_gerbilState = 6004;
		series_stream("604mg06", 4, 0xc00, kCHANGE_GERBILS_ANIMATION);
		series_play("604mg06s", 4, 0xc80);
		player_set_commands_allowed(false);
	}

	switch (_G(flags)[kHampsterState]) {
	case 6000:
		Section6::_state4 = 0;
		break;
	case 6006:
		Section6::_state4 = 5;
		break;
	case 6007:
		Section6::_state4 = 7;
		break;
	default:
		break;
	}

	kernel_trigger_dispatch_now(6014);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 601:
		_sectionSeries1 = series_load("604wi04");
		Section6::_series603 = series_load("604wi04s");
		_G(wilbur_should) = 1;

		ws_demand_location(328, 317, 2);
		ws_hide_walker();
		player_set_commands_allowed(false);

		if (_G(flags)[V242]) {
			_roomSeries1.show("604wi04", 1);
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			kernel_trigger_dispatch_now(0);
		}
		break;

	case 602:
	case 603:
	case 612:
		if (Section6::_state2) {
			_G(wilbur_should) = 7;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 6;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	default:
		_sectionSeries1 = series_load("604wi04");
		Section6::_series603 = series_load("604wi04s");

		ws_demand_location(328, 317, 2);
		ws_hide_walker();
		player_set_commands_allowed(false);
		_roomSeries1.show("604wi04", 1);
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;
	}
}

void Room604::daemon() {
	switch (_G(kernel).trigger) {
	case 0:
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "604intro", 6, 1, 1);
		break;

	case 1:
		digi_unload_stream_breaks(SERIES1);
		_roomSeries1.show("604wi04", 1);
		_woodchipsShould = 25;
		kernel_trigger_dispatch_now(kCHANGE_WOODCHIPS_ANIMATION);
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 2:
		term_message("Kibble is now being removed.");
		inv_move_object("KIBBLE", NOWHERE);
		break;

	case 3:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(4);
		break;

	case 4:
		series_show("602spill", 0x900);
		_G(kernel).call_daemon_every_loop = true;
		kernel_trigger_dispatch_now(2);
		break;

	case 7:
		_G(game).new_room = 602;
		break;

	case 8:
		_G(game).new_room = 603;
		break;

	case 9:
		_G(game).new_room = 612;
		break;

	case 6002:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 7);
		break;

	case 6003:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 8);
		break;

	case 6010:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 9);
		break;

	case kCHANGE_GERBILS_ANIMATION:
		switch (Section6::_gerbilState) {
		case 22:
			break;

		case 23:
			_G(flags)[kHampsterState] = 6006;
			_sectionMachine1 = series_play(_G(executing) == WHOLE_GAME ? "604mg04" : "604mg04a",
				0xcff, 0, kCHANGE_GERBILS_ANIMATION, 8, 0, 100, 0, 0, 0, 50);
			_sectionMachine2 = series_play("604mg04s", 0xd00, 0, -1, 8, 0, 100, 0, 0, 0, 50);
			Section6::_state4 = 3;
			kernel_trigger_dispatch_now(6014);
			Section6::_gerbilState = 24;
			break;

		case 24:
			Section6::_gerbilState = 6003;
			Section6::_state4 = 4;
			kernel_trigger_dispatch_now(6014);
			_sectionMachine1 = series_play(_G(executing) == WHOLE_GAME ? "604mg04" : "604mg04a",
				0xcff, 0, kCHANGE_GERBILS_ANIMATION, 8, 0, 100, 0, 0, 51, -1);
			_sectionMachine2 = series_play("604mg04s", 0xd00, 0, -1, 8, 0, 100, 0, 0, 51, -1);
			break;

		case 6003:
			Section6::_gerbilState = _G(flags)[V245] == 10030 ? 6005 : 6004;
			_sectionMachine1 = series_show("604mg04", 0xcff, 0, -1, -1, 83, 100, 0, 0);
			_sectionMachine2 = series_show("604mg04s", 0xd00, 0, -1, -1, 83, 100, 0, 0);
			kernel_trigger_dispatch_now(kCHANGE_GERBILS_ANIMATION);
			break;

		case 6004:
			if (!_G(flags)[V246])
				_G(game).new_room = 605;
			break;

		case 6005:
			kernel_trigger_dispatch_now(6006);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCHANGE_WOODCHIPS_ANIMATION:
		switch (_woodchipsShould) {
		case 25:
			_woodchips = series_show("604chips", 0xcc0);
			break;

		case 26:
			_woodchipsShould = 28;
			hotspot_set_active("WOOD SHAVINGS", false);
			hotspot_set_active("ASHES", true);

			terminateMachineAndNull(_woodchips);
			digi_play("604_002", 2, 255, -1, 604);
			_woodchips = series_play("604chips", 0xb00, 0, kCHANGE_WOODCHIPS_ANIMATION);
			break;

		case 27:
			_woodchips = series_show("604chips", 0xcc0, 0, -1, -1, 41, 100, 0, 0);
			break;

		case 28:
			_woodchipsShould = 27;
			series_unload(_woodchipSeries);
			kernel_trigger_dispatch_now(kCHANGE_WOODCHIPS_ANIMATION);
			break;

		default:
			break;
		}
		break;

	case 10008:
		if (_G(flags)[kHampsterState] == 6000) {
			_G(flags)[V002] = 1;
			_G(flags)[kHampsterState] = 6006;
			_G(flags)[V248] = 1;
			Section6::_gerbilState = 23;
			kernel_trigger_dispatch_now(kCHANGE_GERBILS_ANIMATION);
			_G(wilbur_should) = 10001;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			_G(wilbur_should) = 2;
			digi_play("604_006", 2, 155, kCHANGE_WILBUR_ANIMATION);
			break;

		case 2:
			_roomSeries1.terminate();
			inv_give_to_player("RAY GUN");
			series_play_with_breaks(PLAY1, "604wi04", 1, kCHANGE_WILBUR_ANIMATION, 3);
			_G(wilbur_should) = 3;
			break;

		case 3:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			switch (_G(flags)[V242]) {
			case 0:
				wilbur_speech("604w001");
				break;
			case 1:
				wilbur_speech("604w002");
				break;
			case 2:
				wilbur_speech("604w003");
				break;
			case 3:
				wilbur_speech("604w004");
				break;
			default:
				wilbur_speech("604w005");
				break;
			}
			break;

		case 4:
			player_set_commands_allowed(false);
			ws_demand_facing(3);
			ws_hide_walker();
			series_play_with_breaks(PLAY2, "604wi06", 0x4ff, 6003, 3);
			break;

		case 5:
			player_set_commands_allowed(false);
			ws_demand_facing(7);
			ws_hide_walker();

			if (_G(flags)[kHampsterState] == 6007) {
				series_play_with_breaks(PLAY3, "604wi07", 0xc80, 6010, 3);
			} else {
				series_play_with_breaks(PLAY3, "604wi07", 0xc80, 6002, 3);
			}

			_G(flags)[V246] = 1;
			break;

		case 6:
			ws_demand_location(171, 310, 3);
			ws_hide_walker();
			player_set_commands_allowed(false);

			if (_G(flags)[kHampsterState] == 6006) {
				Section6::_state4 = 5;
				kernel_trigger_dispatch_now(6014);
				_G(wilbur_should) = 18;
				Section6::_gerbilState = 6004;
				series_stream("604mg06", 4, 0xc80, kCHANGE_GERBILS_ANIMATION);
				series_play("604mg06s", 4, 0xc80, 0, -1);
				series_play_with_breaks(PLAY4, "604wi08", 0x4ff, kCHANGE_WILBUR_ANIMATION, 3);
			} else {
				_G(wilbur_should) = 10001;
				series_play_with_breaks(PLAY4, "604wi08", 0x4ff, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 7:
			_G(wilbur_should) = 10001;
			ws_demand_location(337, 279, 7);
			ws_hide_walker();
			series_play_with_breaks(PLAY5, "604wi09", 0xc80, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 8:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY12, "604wi40", 0x800, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 9:
			ws_hide_walker();
			_G(wilbur_should) = 20;
			series_play_with_breaks(PLAY6, "604wi10", 0xcc0, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 10:
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(false);
			if (_G(executing) == INTERACTIVE_DEMO) {
				series_play_with_breaks(PLAY10Demo, "604wi11", 0xa00, kCHANGE_WILBUR_ANIMATION, 3);
			} else {
				series_play_with_breaks(PLAY10, "604wi11", 0xa00, kCHANGE_WILBUR_ANIMATION, 3);
			}
			// Remove Kibble from inventory
			kernel_trigger_dispatch_now(2);

			if (_G(flags)[V274] == 0 && _G(flags)[kHampsterState] == 6000) {
				_G(flags)[kHampsterState] = 6006;
				_G(flags)[V248] = 1;
				term_message("The gerbils awaken");
				Section6::_gerbilState = 23;
				kernel_trigger_dispatch_now(kCHANGE_GERBILS_ANIMATION);
			}
			break;

		case 11:
			ws_hide_walker();
			player_set_commands_allowed(false);
			_G(flags)[V247] = 1;
			_G(flags)[V274] = 1;
			_woodchipSeries = series_load("604chips");
			_G(wilbur_should) = 12;

			series_play_with_breaks(PLAY11, "604wi12", 0x600, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 12:
			if (_G(flags)[kHampsterState] == 6000) {
				_G(flags)[kHampsterState] = 6006;
				Section6::_gerbilState = 23;
				kernel_trigger_dispatch_now(kCHANGE_GERBILS_ANIMATION);
			}

			_G(wilbur_should) = 10001;
			_woodchipsShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_WOODCHIPS_ANIMATION);
			break;

		case 13:
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY7, "604wi10", 0xcc0, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 14:
			term_message("Slip on kibble!");
			player_set_commands_allowed(false);
			player_update_info();
			_G(wilbur_should) = 15;

			ws_walk(_G(player_info).x + 1, 316, 0, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 15:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 19;
			Section6::_savedX = _G(player_info).x - 358;
			Section6::_state1 = imath_ranged_rand(0, 5);

			series_play_with_breaks(PLAY8, "604wi31", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION,
				3, 6, 100, Section6::_savedX, 0);
			break;

		case 16:
			ws_demand_location(425, 303, 9);
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(flags)[V273] = 1;
			_G(kernel).call_daemon_every_loop = true;

			series_play_with_breaks(PLAY9, "604wi30", 0x901, 3, 3);
			break;

		case 17:
			if (_G(flags)[kHampsterState] == 6000) {
				if (_G(flags)[V248]) {
					_G(wilbur_should) = 11;
					player_set_commands_allowed(false);
					wilbur_speech("604w028", kCHANGE_WILBUR_ANIMATION);
				} else {
					_G(wilbur_should) = 10001;
					wilbur_speech("604w027", kCHANGE_WILBUR_ANIMATION);
				}
			} else {
				_G(wilbur_should) = 11;
				player_set_commands_allowed(false);
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 18:
			ws_unhide_walker();
			player_set_commands_allowed(false);
			_G(wilbur_should) = 21;
			wilbur_speech("604w006", kCHANGE_WILBUR_ANIMATION);
			break;

		case 19:
			_G(wilbur_should) = 10002;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("600w012");
			break;

		case 20:
			Section6::_gerbilState = 23;
			kernel_trigger_dispatch_now(kCHANGE_GERBILS_ANIMATION);
			ws_unhide_walker();

			if (_G(flags)[V248]) {
				digi_preload("604w014");
				player_set_commands_allowed(true);
				_G(wilbur_should) = 10002;
				wilbur_speech("604w014", kCHANGE_WILBUR_ANIMATION);
			} else {
				digi_preload("604w012");
				player_set_commands_allowed(false);
				_G(wilbur_should) = 10001;
				_G(flags)[V248] = 1;
				wilbur_speech("604w012", kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 21:
			player_set_commands_allowed(false);
			ws_unhide_walker();
			break;

		case 10003:
			if (_G(flags)[kHampsterState] == 6000) {
				series_load("604mg04");
				series_load("604mg04s");
			}

			_G(kernel).continue_handling_trigger = true;
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCALLED_EACH_LOOP:
		player_update_info();
		if (_G(player_info).x > 319 && _G(player_info).x < 413 &&
				_G(player_info).y > 280 && _G(player_info).y < 305 &&
				_G(player_info).facing > 2 && _G(player_info).facing < 7 &&
				_G(flags)[V273] == 1) {
			if (_G(flags)[kStandingOnKibble]) {
				_G(flags)[kStandingOnKibble] = 1;
			} else {
				_G(flags)[kStandingOnKibble] = 1;
				intr_cancel_sentence();
				_G(wilbur_should) = 14;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}
		} else {
			_G(flags)[kStandingOnKibble] = 0;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room604::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[kHampsterState] == 6006 && player_said("TUBE ")) {
		intr_cancel_sentence();
		wilbur_speech("600w003");

	} else if (player_said("RAY GUN", "WOOD SHAVINGS")) {
		_G(player).command_ready = false;

		if (_G(flags)[V274] == 0) {
			_G(wilbur_should) = 17;
			player_hotspot_walk_override(353, 313, 10, kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("WOOD SHAVINGS", "KIBBLE")) {
		_G(wilbur_should) = 10;
		player_hotspot_walk_override(308, 301, 10, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "GERBILS")) {
		_G(wilbur_should) = 8;
		player_hotspot_walk_override(286, 297, 10, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("KIBBLE", "FLOOR")) {
		if (_G(flags)[V273]) {
			_G(player).need_to_walk = false;
			_G(player).need_to_walk = false;
			wilbur_speech("600w008z");
		} else {
			_G(wilbur_should) = 16;
			player_hotspot_walk_override(425, 303, 9, kCHANGE_WILBUR_ANIMATION);
			_G(player).command_ready = false;
		}

		return;
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room604::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool rayGun = player_said("RAY GUN");

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "WOOD SHAVINGS")) {
		player_set_commands_allowed(false);

		if (_G(flags)[kHampsterState] == 6000) {
			_G(wilbur_should) = 9;
			wilbur_speech(_G(flags)[V248] ? "604w013" : "604w011",
				kCHANGE_WILBUR_ANIMATION);

		} else {
			_G(wilbur_should) = 13;
			wilbur_speech("604w005", kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("GEAR", "TUBE ") || player_said("CLIMB IN", "TUBE ")) {
		_G(flags)[V246] = 1;
		Section6::_state2 = 2;
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "TUBE") || player_said("CLIMB IN", "TUBE")) {
		_G(flags)[V246] = 1;
		Section6::_state2 = 0;
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (rayGun && player_said("APPLE CORE")) {
		wilbur_speech("604w029");
	} else if (rayGun && player_said("ASHES")) {
		wilbur_speech("600w004");
	} else if (rayGun && player_said("BOTTLE CAP")) {
		wilbur_speech("600w004");
	} else if (player_said("LOOK AT", "WOOD SHAVINGS")) {
		if (_G(flags)[V248] == 0) {
			wilbur_speech("604w007");
		} else if (_G(flags)[kHampsterState] == 6000) {
			wilbur_speech("604w008");
		} else {
			wilbur_speech("604w005");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
