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

#include "m4/burger/rooms/section1/room143.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_CAT_ANIMATION = 43,
	kCHANGE_VERA_ANIMATION = 44,
	kCHANGE_BURL_ANIMATION = 45
};

const char *Room143::SAID[][4] = {
	{ "VERA",          "143W001", "999w011", "999w011" },
	{ "BURL",          nullptr,   "999w011", "999w011" },
	{ "SWINGING DOOR", "143w006", "999w011", nullptr   },
	{ "ORDER WINDOW",  "143w006", "999w011", "999w011" },
	{ "JUKEBOX",       "143w008", nullptr,   nullptr   },
	{ "OUTSIDE",       nullptr,   "999w011", nullptr   },
	{ "MUFFIN",        "143w009", nullptr,   nullptr   },
	{ "FOUNTAIN",      "143W010", nullptr,   nullptr   },
	{ "MOOSEHEAD",     "143W011", nullptr,   nullptr   },
	{ "MENU",          "143W012", nullptr,   nullptr   },
	{ "MOUSE TRAP",    "143W013", nullptr,   "999w011" },
	{ "STOOL",         nullptr,   nullptr,   "143W017" },
	{ "BOOTH",         nullptr,   nullptr,   "143W017" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room143::PLAY1[] = {
	{  0, 20, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 21, 21, "405_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 22, 28, "405w015", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, -1, "143_008", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY2[] = {
	{ 0,  5, nullptr, 0,   0, -1, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 255, 12, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY3[] = {
	{  0, 76, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 77, -1, nullptr, 2, 0,  3, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY4[] = {
	{ 0, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY5[] = {
	{ 0, 3, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY6[] = {
	{  0,  2, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  3, 14, "143_006", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 15, 25, "143_005", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 26, 29, "143_007", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 30, 30, nullptr,   0,   0, 40, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY7[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY8[] = {
	{  0, 22, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 24, -1, nullptr, 0, 0,  8, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY9[] = {
	{  0, 25, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 26, -1, "143_004", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY10[] = {
	{  0,  6, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{  6,  6, nullptr,   0,   0, 32,    0, 0, nullptr, 0 },
	{  7, 12, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 16, "143b007", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 17, 21, "143b009", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 22, 23, nullptr,   1, 255, -1, 2048, 0, nullptr, 0 },
	{ 24, -1, "143b010", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY11[] = {
	{ 0, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr, 0, 0, 36, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room143::init() {
	_frame = 0;
	_flag1 = false;
	digi_preload("143_001");
	_G(kernel).call_daemon_every_loop = true;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 142:
		player_set_commands_allowed(false);
		ws_demand_location(148, 247, 3);
		ws_walk(249, 252, 0, 13, 4);
		break;

	default:
		ws_demand_location(290, 300, 5);
		break;
	}

	_plate = series_play("143plate", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	if (_G(flags)[V000] == 1003 && _G(flags)[V063])
		digi_preload("143_002");

	_veraShould = 8;
	_veraMode = 8;
	kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
	_val3 = 0;
	_catShould = 6;
	kernel_trigger_dispatch_now(kCHANGE_CAT_ANIMATION);

	if (_G(flags)[V064] == 1)
		loadCheese();
	else
		hotspot_set_active("cheese", false);

	if (_G(flags)[V000] == 1003) {
		if (_G(flags)[V063]) {
			_burlMode = _burlShould = 42;
		} else {
			_burlMode = _burlShould = 31;
		}

		kernel_trigger_dispatch_now(kCHANGE_BURL_ANIMATION);
	} else {
		hotspot_set_active("burl", false);
	}

	if (inv_object_in_scene("BROKEN MOUSE TRAP", 143)) {
		_walk1 = intr_add_no_walk_rect(298, 310, 348, 343, 285, 314);
		_mouseTrap = series_play("143mtrap", 0xf00, 0, -1, 600, -1, 100, 25, 0, 0, 0);
	} else {
		hotspot_set_active("MOUSE TRAP", false);
	}

	if (_G(flags)[V000] == 1004) {
		showEmptyPlates();
		series_play("143money", 0xf02, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	} else {
		hotspot_set_active("money ", false);
	}

	digi_play_loop("143_001", 3, 255, -1);
}

void Room143::daemon() {
	int frame;

	if (_G(flags)[V064] == 1) {
		if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
			player_update_info();

			if (_G(player_info).y < 260) {
				_G(flags)[V064] = 2;
				kernel_timing_trigger(1, 5);
			}
		}
	}

	if (_G(flags)[V063] && !digi_play_state(2))
		digi_play_loop("143_002", 2, 255, -1);

	if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE && _flag1) {
		player_update_info();

		if (_G(player_info).y > 324)
			ws_demand_location(_G(player_info).x, 324);
	}

	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 2:
		ws_unhide_walker();
		break;

	case 3:
		digi_play("143_010", 2, 255, 4);
		break;

	case 4:
		digi_unload("143_010");
		break;

	case 5:
		terminateMachineAndNull(_cheese);
		hotspot_set_active("cheese", false);
		_flag1 = true;
		digi_preload("143_010", false);
		series_play_with_breaks(PLAY3, "143mous", 0xf00, 6, 3, 4, 100, 35, -5);
		break;

	case 6:
		_flag1 = false;
		break;

	case 7:
		digi_play("143e600", 1, 255, 1);
		_veraShould = 8;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
		break;

	case 8:
		hotspot_set_active("money ", true);
		series_play("143money", 0xf02, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		break;

	case 10:
		player_set_commands_allowed(true);
		break;

	case 11:
		_burlShould = (_burlMode == 42) ? 42 : 31;
		player_set_commands_allowed(true);
		break;

	case 12:
		_G(flags)[V063] = 1;
		break;

	case 13:
		if (_G(player).been_here_before) {
			if (inv_object_is_here("broken mouse trap")) {
				wilbur_speech("143w507", 14);
			} else {
				_digiName = "143v508";
				_veraShould = 11;
				_digiMode = KT_DAEMON;
				_digiTrigger = 15;
			}
		} else {
			wilbur_speech("143w506", 16);
		}
		break;

	case 14:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			_digiName = "143v509a";
			break;
		case 2:
			_digiName = "143v509b";
			break;
		default:
			_digiName = "143v509c";
			break;
		}

		_veraShould = 11;
		_digiMode = KT_DAEMON;
		_digiTrigger = 15;
		break;

	case 15:
		_veraShould = 8;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
		player_set_commands_allowed(true);
		break;

	case 16:
		_digiName = "143v506";
		_veraShould = 22;
		_digiMode = KT_DAEMON;
		_digiTrigger = 17;
		break;

	case 17:
		_veraShould = 13;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
		digi_play("143e503", 1, 255, 18);
		break;

	case 18:
		_digiName = "143v507";
		_veraShould = 14;
		_digiMode = KT_DAEMON;
		_digiTrigger = 15;
		break;

	case 19:
		// Burl turns to talk to Wilbur
		_burlShould = 37;
		break;

	case 21:
		_catShould = 7;
		kernel_trigger_dispatch_now(kCHANGE_CAT_ANIMATION);
		break;

	case 22:
		_digiName = "143b001";
		_digiMode = KT_DAEMON;
		_digiTrigger = 23;
		_burlShould = 30;
		_burlMode = 29;
		_veraShould = 21;
		kernel_trigger_dispatch_now(kCHANGE_BURL_ANIMATION);
		break;

	case 23:
		_digiName = "143v901";
		_digiMode = KT_DAEMON;
		_digiTrigger = 24;
		_burlShould = 29;
		_veraShould = 22;
		break;

	case 24:
		_digiName = "143b002";
		_digiMode = KT_DAEMON;
		_digiTrigger = 25;
		_burlShould = 30;
		_veraShould = 21;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
		break;

	case 25:
		_digiName = "143v902";
		_digiMode = KT_DAEMON;
		_digiTrigger = 26;
		_burlShould = 29;
		_veraShould = 22;
		break;

	case 26:
		_digiName = "143v903";
		_digiMode = KT_DAEMON;
		_digiTrigger = 7;
		_veraShould = 14;
		_burlShould = 31;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
		break;

	case 28:
		disable_player();
		digi_play("143w610", 1, 255, 29);
		series_play("143wi03", 0x100, 0, 31, 8, 0, 100, 0, 0, 0, 10);
		break;

	case 29:
		_digiName = "143b006";
		_digiMode = KT_DAEMON;
		_digiTrigger = 30;
		_burlShould = 36;
		break;

	case 30:
		_burlShould = 39;
		break;

	case 31:
		_wi03 = series_play("143wi03", 0x100, 0, -1, 600, -1, 100, 0, 0, 10, 10);
		break;

	case 32:
		terminateMachineAndNull(_wi03);
		series_play("143wi03", 0x100, 0, 33, 6, 0, 100, 0, 0, 11, -1);
		break;

	case 33:
		ws_unhide_walker();
		break;

	case 34:
		disable_player();
		Series::series_play("143wi03", 0x100, 0, 35, 8, 0, 100, 0, 0, 0, 10);
		break;

	case 35:
		_wi03 = series_play("143wi03", 0x100, 0, -1, 600, -1, 100, 0, 0, 10, 10);
		_wi03S = series_play("143wi03s", 0x101, 0, -1, 600, -1, 100, 0, 0, 10, 10);
		_digiName = "143b005";
		_digiMode = KT_DAEMON;
		_digiTrigger = 39;
		_burlShould = (_burlMode == 42) ? 44 : 36;
		break;

	case 38:
		enable_player();
		break;

	case 39:
		terminateMachineAndNull(_wi03);
		terminateMachineAndNull(_wi03S);
		Series::series_play("143wi03", 0x100, 0, 38, 6, 0, 100, 0, 0, 11, -1);
		_burlShould = (_burlMode == 42) ? 42 : 31;
		break;

	case 40:
		_digiName = "143v904";
		_digiMode = KT_DAEMON;
		_digiTrigger = 41;
		_burlShould = 35;
		_veraShould = 11;
		break;

	case 41:
		_burlShould = 34;
		_digiName = "143b003";
		_digiMode = KT_DAEMON;
		_digiTrigger = 42;
		_veraShould = 8;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
		break;

	case 42:
		_burlShould = 42;
		break;

	case kCHANGE_CAT_ANIMATION:
		switch (_catShould) {
		case 6:
			_cat.show("143cat", 0);
			break;

		case 7:
			_cat.terminate();
			_catShould = 6;
			series_play_with_breaks(PLAY4, "143cat", 0, kCHANGE_CAT_ANIMATION, 3, 10, 100, 0, 0);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_VERA_ANIMATION:
		switch (_veraMode) {
		case 8:
			switch (_veraShould) {
			case 8:
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					series_play_with_breaks(PLAY5, "143ve02", 0xf00, kCHANGE_VERA_ANIMATION, 2);
					break;

				case 2:
					_veraMode = 10;
					series_play("143ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
					break;

				default:
					series_play("143ve01", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
					break;
				}
				break;

			case 11:
				_veraMode = 10;
				series_play("143ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 13:
			case 14:
			case 15:
				_veraMode = 13;
				Series::series_play("143ve14", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 8);
				break;

			case 20:
				_veraShould = 8;
				series_play_with_breaks(PLAY6, "143ve07", 0xf00, kCHANGE_VERA_ANIMATION, 3);
				break;

			case 21:
			case 22:
				_veraMode = 21;
				Series::series_play("143ve08", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 25:
			case 26:
				_veraMode = 27;
				series_play("143ve04", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 5);
				break;

			default:
				break;
			}
			break;

		case 9:
			if (_veraShould == 8) {
				if (imath_ranged_rand(1, 20) == 1) {
					_veraMode = 21;
					Series::series_play("143ve10", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				} else {
					Series::series_play("143ve10", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 3, 3);
				}
			} else {
				_veraMode = 21;
				Series::series_play("143ve10", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
			}
			break;

		case 10:
			switch (_veraShould) {
			case 8:
				if (imath_ranged_rand(1, 20) == 1) {
					_veraMode = 8;
					series_play("143ve03", 0xf00, 2, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				} else {
					series_play("143ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 3, 3);
				}
				break;

			case 11:
				_veraMode = 11;
				series_play("143ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, 3, 3);
				break;

			default:
				_veraMode = 8;
				series_play("143ve03", 0xf00, 2, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				break;
			}
			break;

		case 11:
			if (_veraShould == 11) {
				_ve03 = series_play("143ve03", 0xf00, 4, -1, 4, -1, 100, 0, 0, 3, 6);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				_veraMode = 10;
				kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			}
			break;

		case 13:
			switch (_veraShould) {
			case 13:
				Series::series_play("143ve15", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 4, 4);
				break;

			case 14:
				_veraMode = 14;
				Series::series_play("143ve17", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 2);
				break;

			case 15:
				_veraMode = (imath_ranged_rand(1, 2) == 1) ? 16 : 17;
				Series::series_play("143ve18", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			case 18:
				_veraShould = 19;
				Series::series_play("143ve16", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 1);
				break;

			case 19:
				terminateMachineAndNull(_plate);
				_veraShould = 13;
				Series::series_play("143ve16", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 2, 15);
				break;

			case 21:
				_veraMode = 21;
				Series::series_play("143ve15", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			default:
				_veraMode = 10;
				Series::series_play("143ve19", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, -1);
				break;
			}
			break;

		case 14:
			if (_veraShould == 14) {
				_ve03 = series_play("143ve18", 0xf00, 4, -1, 4, -1, 100, 0, 0, 1, 4);
				_ve03S = series_play("143ve18s", 0xf01, 4, -1, 4, -1, 100, 0, 0, 1, 4);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				terminateMachineAndNull(_ve03S);
				_veraMode = 13;
				Series::series_play("143ve18", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 16:
			if (_veraShould == 15) {
				_ve03 = series_play("143ve17", 0xf00, 4, -1, 4, -1, 100, 0, 0, 3, 5);
				_ve03S = series_play("143ve17s", 0xf01, 4, -1, 4, -1, 100, 0, 0, 3, 5);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				terminateMachineAndNull(_ve03S);
				_veraMode = 13;
				Series::series_play("143ve18", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 2);
			}
			break;

		case 17:
			if (_veraShould == 15) {
				_ve03 = series_play("143ve18", 0xf00, 4, -1, 4, -1, 100, 0, 0, 5, 8);
				_ve03S = series_play("143ve18s", 0xf01, 4, -1, 4, -1, 100, 0, 0, 5, 8);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				terminateMachineAndNull(_ve03S);
				_veraMode = 13;
				Series::series_play("143ve18", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
			}
			break;


		case 21:
			switch (_veraShould) {
			case 15:
				_veraMode = 13;
				Series::series_play("143ve15", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 21:
				if (imath_ranged_rand(1, 20) == 1) {
					_veraMode = 9;
					Series::series_play("143ve10", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				} else {
					Series::series_play("143ve08", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 4, 4);
				}
				break;

			case 22:
				if (imath_ranged_rand(1, 2) == 1) {
					_veraMode = 23;
					kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
				} else {
					_veraMode = 24;
					Series::series_play("143ve11", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 2, 4);
				}
				break;

			default:
				_veraMode = 8;
				Series::series_play("143ve08", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 23:
			if (_veraShould == 22) {
				_ve03 = series_play("143ve11", 0xf00, 0, -1, 6, -1, 100, 0, 0, 0, 1);
				_ve03S = series_play("143ve11s", 0xf01, 0, -1, 6, -1, 100, 0, 0, 0, 1);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				terminateMachineAndNull(_ve03S);
				_veraMode = 21;
				kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			}
			break;

		case 24:
			if (_veraShould == 22) {
				_ve03 = series_play("143ve11", 0xf00, 4, -1, 6, -1, 100, 0, 0, 4, 6);
				_ve03S = series_play("143ve11s", 0xf01, 4, -1, 6, -1, 100, 0, 0, 4, 6);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				terminateMachineAndNull(_ve03S);
				_veraMode = 21;
				Series::series_play("143ve11", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 2, 4);
			}
			break;

		case 25:
			switch (_veraShould) {
			case 25:
				Series::series_play("143ve06", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 26:
				_veraMode = 26;
				Series::series_play("143ve06", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, 0, 0);
				break;

			default:
				_veraMode = 28;
				Series::series_play("143ve05", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 26:
			if (_veraShould == 26) {
				_ve03 = series_play("143ve06", 0xf00, 4, -1, 5, -1, 100, 0, 0, 0, 4);
				_ve03S = series_play("143ve06s", 0xf01, 4, -1, 5, -1, 100, 0, 0, 0, 4);
				playDigi2();
			} else {
				terminateMachineAndNull(_ve03);
				terminateMachineAndNull(_ve03S);
				_veraMode = 25;
				kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			}
			break;

		case 27:
			_veraMode = 25;
			Series::series_play("143ve05", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
			break;

		case 28:
			_veraMode = 8;
			Series::series_play("143ve04", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 5);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_BURL_ANIMATION:
		switch (_burlMode) {
		case 20:
			_burlMode = 31;
			kernel_trigger_dispatch_now(kCHANGE_BURL_ANIMATION);
			break;

		case 29:
			switch (_burlShould) {
			case 29:
				Series::series_play("143bu02", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			case 30:
				frame = imath_ranged_rand(0, 5);
				Series::series_play("143bu03", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, frame, frame);
				playDigi1();
				break;

			default:
				_G(flags)[kDisableFootsteps] = 0;
				_burlMode = 20;
				series_play_with_breaks(PLAY9, "143bu04", 0xa00, kCHANGE_BURL_ANIMATION, 3);
				break;
			}
			break;

		case 31:
			switch (_burlShould) {
			case 31: {
				int rand = imath_ranged_rand(1, 20);

				if (_val3 != -1) {
					++_val3;

					if (imath_ranged_rand(10, 20) >= _val3)
						rand = 2;
					else
						_val3 = -1;
				}

				switch (rand) {
				case 1:
					_burlMode = 47;
					Series::series_play("143bu10", 0xa00, 0, kCHANGE_BURL_ANIMATION);
					break;

				case 2:
					if (_val3 == -1)
						_val3 = 0;

					Series::series_play("143bu05", 0xa00, 0, kCHANGE_BURL_ANIMATION);
					break;

				default:
					Series::series_play("143bu05", 0xa00, 0, kCHANGE_BURL_ANIMATION, 20, 0, 100, 0, 0, 0, 0);
					break;
				}
				break;
			}

			case 33:
				_burlShould = 31;
				series_play_with_breaks(PLAY11, "143bu11", 0xa00, kCHANGE_BURL_ANIMATION, 3);
				break;

			case 35:
				_burlMode = 35;
				Series::series_play("143bu06", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 36:
			case 37:
			case 38:
			case 39:
				_burlMode = 37;
				Series::series_play("143bu11", 0xa00, 0, kCHANGE_BURL_ANIMATION);
				break;

			case 45:
				series_load("143money");
				series_play_with_breaks(PLAY8, "143bu24", 0xe00, 1, 3);
				break;

			default:
				_burlMode = 41;
				Series::series_play("143bu17", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				break;
			}
			break;

		case 32:
			digi_stop(2);
			_G(flags)[V063] = 0;

			_burlMode = 31;
			showEmptyPlates();
			Series::series_play("143bu25", 0xa00, 2, kCHANGE_BURL_ANIMATION);
			break;

		case 35:
			switch (_burlShould) {
			case 34:
				Series::series_play("143bu06", 0xa00, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 3, 3);
				playDigi1();
				break;

			case 35:
				Series::series_play("143bu06", 0xa00, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 3, 3);
				break;

			default:
				_burlMode = 31;
				Series::series_play("143bu06", 0xa00, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				break;
			}
			break;

		case 37:
			switch (_burlShould) {
			case 36:
				frame = imath_ranged_rand(0, 5);
				Series::series_play("143bu13", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, frame, frame);
				playDigi1();
				break;

			case 37:
				if (imath_ranged_rand(1, 60) == 22) {
					// Talk about unlikely chance of happening
					Series::series_play("143bu14", 0xa00, 0, kCHANGE_BURL_ANIMATION);

					switch (imath_ranged_rand(1, 4)) {
					case 1:
						digi_play("143b007", 2, 255, -1);
						break;
					case 2:
						digi_play("143b011", 2, 255, -1);
						break;
					case 3:
						digi_play("143b012", 2, 255, -1);
						break;
					case 4:
						digi_play("143b013", 2, 255, -1);
						break;
					default:
						break;
					}
				} else {
					Series::series_play("143bu13", 0xa00, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				}
				break;

			case 39:
				_burlShould = 40;
				series_play_with_breaks(PLAY10, "143bu15", 0xa00, kCHANGE_BURL_ANIMATION);
				break;

			case 40:
				digi_play("143b008", 1, 255, 1);
				_burlShould = 31;
				kernel_trigger_dispatch_now(kCHANGE_BURL_ANIMATION);
				break;

			default:
				_burlMode = 31;
				Series::series_play("143bu11", 0xa00, 2, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				break;
			}
			break;

		case 41:
			_burlMode = 42;
			digi_preload("143_002");
			player_set_commands_allowed(true);
			series_play_with_breaks(PLAY2, "143bu18", 0xa00, kCHANGE_BURL_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 42:
			switch (_burlShould) {
			case 42:
				++_val3;

				if (imath_ranged_rand(10, 15) >= _val3) {
					do {
						if (_frame >= 5)
							_frame = 0;
						frame = imath_ranged_rand(0, 5);
					} while (frame <= _frame);

					_frame = frame;
					Series::series_play("143bu19", 0xa00, 0, kCHANGE_BURL_ANIMATION, 8, 0, 100, 0, 0, frame, frame);

				} else if (imath_ranged_rand(1, 30) == 1) {
					Series::series_play("143bu19", 0xa00, 0, kCHANGE_BURL_ANIMATION, 8, 0, 100, 0, 0, 6, 12);
				} else {
					do {
						if (_frame >= 5)
							_frame = 0;
						frame = imath_ranged_rand(0, 5);
					} while (frame <= _frame);

					_frame = frame;
					Series::series_play("143bu19", 0xa00, 0, kCHANGE_BURL_ANIMATION, 8, 0, 100, 0, 0, frame, frame);
				}
				break;

			case 44:
				_burlMode = 43;
				Series::series_play("143bu20", 0xa00, 0, kCHANGE_BURL_ANIMATION, 4);
				break;

			case 45:
				_G(flags)[V063] = 0;
				_burlMode = 46;
				Series::series_play("143bu19", 0xa00, 0, kCHANGE_BURL_ANIMATION, 4, 0, 100, 0, 0, 13, -1);
				break;

			default:
				_G(flags)[V063] = 0;
				_burlMode = 32;
				Series::series_play("143bu19", 0xa00, 0, kCHANGE_BURL_ANIMATION, 4, 0, 100, 0, 0, 13, -1);
				break;
			}
			break;

		case 43:
			switch (_burlShould) {
			case 43:
				frame = imath_ranged_rand(0, 5);
				Series::series_play("143bu21", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, frame, frame);
				break;

			case 44:
				frame = imath_ranged_rand(0, 4);
				Series::series_play("143bu22", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, frame, frame);
				playDigi1();
				break;

			default:
				_burlMode = 42;
				Series::series_play("143bu20", 0xa00, 2, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				break;
			}
			break;

		case 46:
			digi_stop(3);
			showEmptyPlates();
			series_load("143money");
			series_play_with_breaks(PLAY8, "143bu24", 0xe00, 1, 3);
			break;

		case 47:
			_burlMode = 31;
			Series::series_play("143bu10", 0xa00, 0, kCHANGE_BURL_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 2:
			terminateMachineAndNull(_mouseTrap);
			disable_player();
			_G(wilbur_should) = 3;
			series_play_with_breaks(PLAY1, "143wi02", 0xf00, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 25, 0);
			_veraShould = 21;
			break;

		case 3:
			ws_unhide_walker();
			intr_remove_no_walk_rect(_walk1);
			inv_move_object("BROKEN MOUSE TRAP", NOWHERE);
			inv_give_to_player("SPRING");
			hotspot_set_active("MOUSE TRAP", false);

			_veraShould = 22;
			_digiName = "143v501";
			_digiMode = KT_DAEMON;
			_G(wilbur_should) = 5;
			_digiTrigger = kCHANGE_WILBUR_ANIMATION;
			_G(flags)[V064] = 1;
			loadCheese();
			hotspot_set_active("cheese", true);
			break;

		case 5:
			_veraShould = 8;
			kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			player_set_commands_allowed(true);
			break;

		case 10017:
			_G(kernel).continue_handling_trigger = true;
			if (_G(flags)[V064] == 1)
				_G(flags)[V064] = 2;
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kBurlEntersTown:
		if (_G(flags)[kRoadOpened]) {
			_G(kernel).continue_handling_trigger = true;
		} else if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
			_G(flags)[V000] = 1003;
			hotspot_set_active("burl", true);
			player_set_commands_allowed(false);
			intr_freshen_sentence();
			Section1::walk();

			player_update_info();
			if (_G(player_info).y < 304) {
				ws_walk(220, 304, 0, -1, 2);
				_G(flags)[kDisableFootsteps] = 1;
			}

			series_play_with_breaks(PLAY7, "143bu01", 0xe00, 22, 3);
		} else {
			kernel_timing_trigger(60, kBurlEntersTown);
		}
		break;

	case kBurlGetsFed:
		if (_G(flags)[V000] == 1002) {
			_G(kernel).continue_handling_trigger = true;
		} else if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
			_veraShould = 20;
			player_set_commands_allowed(false);
			intr_freshen_sentence();
			Section1::walk();
		} else {
			kernel_timing_trigger(60, kBurlGetsFed);
		}
		break;

	case kBurlStopsEating:
		if (_G(flags)[V000] == 1002) {
			_G(kernel).continue_handling_trigger = true;
		} else if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
			_burlShould = 31;
		} else {
			kernel_timing_trigger(60, kBurlStopsEating);
		}
		break;

	case kBurlLeavesTown:
		if (_G(flags)[V000] == 1002) {
			_G(kernel).continue_handling_trigger = true;
		} else if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
			_G(kernel).continue_handling_trigger = true;
			player_update_info();
			player_set_commands_allowed(false);
			intr_freshen_sentence();
			Section1::walk();

			if (_G(player_info).y < 300)
				ws_walk(213, 287, 0, -1, 2);

			showEmptyPlates();
			_burlShould = 45;
			hotspot_set_active("burl", false);
		} else {
			kernel_timing_trigger(60, kBurlLeavesTown);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room143::pre_parser() {
	if (player_said("outside") && !player_said_any("exit", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9, 0);
}

void Room143::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool money = player_said("money ");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv35")) {
		// Talking to Vera
		conv35();
	} else if (player_said("conv30")) {
		// Talking to Burl
		conv30();
	} else if (money && lookFlag) {
		wilbur_speech("143w030");
	} else if (money && player_said_any("take", "gear")) {
		wilbur_speech("143w031");
	} else if (money && player_said("take")) {
		wilbur_speech("143w030");
	} else if (player_said("take", "mouse trap")) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("talk to", "vera")) {
		talkToVera();
	} else if (player_said("talk to", "burl")) {
		if (player_commands_allowed() || !INTERFACE_VISIBLE) {
			talkToBurl();

			if (_burlMode != 42 && _burlMode != 43) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(60, 19);
			}
		}
	} else if (player_said("outside") && (lookFlag || player_said_any("exit", "gear"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1014);
	} else if (player_said("laxative", "burl")) {
		kernel_trigger_dispatch_now(28);
	} else if (player_said("burl") && inv_player_has(_G(player).verb)) {
		kernel_trigger_dispatch_now(24);
	} else if (player_said("vera") && inv_player_has(_G(player).verb)) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_veraShould = 22;
			_digiName = "143v511";
			_digiMode = KT_PARSE;
			_digiTrigger = 1;
			break;

		case 1:
			player_set_commands_allowed(false);
			_veraShould = 8;
			kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			break;

		default:
			break;
		}
	} else if (player_said("burl") && inv_player_has(_G(player).verb)) {
		// This seems a duplicate of a prior check
		wilbur_speech("143w005");
	} else if (player_said("swinging door") && player_said_any("enter", "gear")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_veraShould = 11;
			_digiName = "143v504";
			_digiMode = KT_PARSE;
			_digiTrigger = 1;
			break;

		case 1:
			_veraShould = 8;
			kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			digi_play("143e502", 1, 255, 1);
			break;

		default:
			break;
		}
	} else if (player_said("quarter", "jukebox") || player_said("gear", "jukebox")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_veraShould = 22;
			_digiName = "143v510";
			_digiMode = KT_PARSE;
			_digiTrigger = 1;
			_G(flags)[V062] = 1;
			break;

		case 1:
			_veraShould = 8;
			kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (player_said("muffin") &&
			(player_said_any("take", "gear") || inv_player_has(_G(player).verb))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_veraShould = 26;
			_digiName = "143v502";
			_digiMode = KT_PARSE;
			_digiTrigger = 1;
			_catShould = 21;
			kernel_trigger_dispatch_now(kCHANGE_CAT_ANIMATION);
			break;

		case 1:
			_veraShould = 13;
			kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			_G(kernel).trigger_mode = KT_PARSE;
			digi_play("143e501", 1, 255, 2);
			break;

		case 2:
			_veraShould = 14;
			_digiName = "143V503";
			_digiMode = KT_PARSE;
			_digiTrigger = 3;
			break;

		case 3:
			player_set_commands_allowed(true);
			_veraShould = 8;
			kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("burl")) {
		if (_burlMode == 42 || _burlMode == 43 || _burlMode == 44)
			wilbur_speech("143w004");
		else
			wilbur_speech("143w003");
	} else if (!_G(walker).wilbur_said(SAID)) {
		return;
	}

	_G(player).command_ready = false;
}

void Room143::conv35() {
	_G(kernel).trigger_mode = KT_PARSE;
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	switch (_G(kernel).trigger) {
	case 9:
		if (who <= 0) {
			if ((node == 9 && entry == 1) || (node == 5 && entry == 1) ||
					(node == 17 && entry == 0) || (node == 19 && entry == 0)) {
				terminateMachineAndNull(_eu02);
				series_play("143eu02", 0xf00, 2, -1, 4, 0, 100, 0, 0, 0, 3);
			}

			if (node == 11 && entry == 0) {
				_veraShould = 13;
			} else if ((node == 1 && entry == 4) ||
					(node == 4 && entry == 0) ||
					(node == 5 && entry == 1) ||
					(node == 7 && entry == 0) ||
					(node == 7 && entry == 1) ||
					(node == 9 && entry == 1) ||
					(node == 10 && entry == 1) ||
					(node == 11 && entry == 0) ||
					(node == 12 && entry == 2) ||
					(node == 13 && entry == 2) ||
					(node == 13 && entry == 1) ||
					(node == 15) ||
					(node == 16 && entry == 0) ||
					(node == 16 && entry == 2) ||
					(node == 17 && entry == 0) ||
					(node == 18 && entry == 0) ||
					(node == 19 && entry == 0)) {
				// Do nothing
			} else {
				_veraShould = 8;
				if (_veraMode != 13) {
					_G(kernel).trigger_mode = KT_DAEMON;
					kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
				}
			}

			conv_resume();
		} else if (who == 1) {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			conv_resume();
		}
		break;

	case 27:
		_eu02 = series_play("143eu02", 0xf00, 0, -1, 600, -1, 100, 0, 0, 3, 3);
		break;

	default:
		if (sound) {
			if (who <= 0) {
				if (node == 5 && entry == 3) {
					_G(kernel).trigger_mode = KT_DAEMON;
					kernel_timing_trigger(300, 21);
					_G(kernel).trigger_mode = KT_PARSE;
				}

				if ((node == 9 && entry == 1) || (node == 5 && entry == 1) ||
						(node == 17 && entry == 0) || (node == 19 && entry == 0)) {
					series_play("143eu02", 0xf00, 0, 27, 4, 0, 100, 0, 0, 0, 3);
				}

				if ((node == 1 && entry == 4) ||
						(node == 4 && entry == 0) ||
						(node == 5 && entry == 1) ||
						(node == 7 && entry == 0) ||
						(node == 7 && entry == 1) ||
						(node == 9 && entry == 1) ||
						(node == 10 && entry == 1) ||
						(node == 11 && entry == 0) ||
						(node == 12 && entry == 2) ||
						(node == 13 && entry == 1) ||
						(node == 15) ||
						(node == 16 && entry == 0) ||
						(node == 16 && entry == 2) ||
						(node == 17 && entry == 0) ||
						(node == 18 && entry == 0) ||
						(node == 19 && entry == 0)) {
					digi_play(sound, 1, 255, 9);
				} else if (node == 3) {
					_burlShould = 34;
					_digiName = sound;
				} else if ((node == 9 && entry == 0) || (node == 17 && entry == 1)) {
					_veraShould = 15;
					_digiName = sound;
				} else if (node == 5 && entry == 3) {
					_veraShould = 26;
					_digiName = sound;
				} else if ((node == 5 && entry == 0) ||
						(node == 1 && entry == 7) ||
						(node == 11 && entry == 1) ||
						(node == 18 && entry == 1) ||
						(node == 8) ||
						(node == 9 && entry == 0) ||
						(node == 10 && entry == 0)) {
					_veraShould = 14;
					_digiName = sound;
				} else {
					_veraShould = 11;
					_digiName = sound;
				}
			} else if (who == 1) {
				sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
				digi_play(sound, 1, 255, 9);
			}

			_digiMode = KT_PARSE;
			_digiTrigger = 9;
		} else {
			conv_resume();
		}
		break;
	}
}

void Room143::conv30() {
	_G(kernel).trigger_mode = KT_PARSE;
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();

	if (_G(kernel).trigger == 9) {
		if (who <= 0) {
			if (_burlMode == 37 || _burlMode == 31) {
				_burlShould = 37;
			} else if (_burlMode == 42 || _burlMode == 43) {
				_burlShould = 43;
				digi_change_volume(2, 255);
			}

			conv_resume();
		} else if (who == 1) {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			if (_burlMode == 37 || _burlMode == 31) {
				_burlShould = 36;
			} else if (_burlMode == 42 || _burlMode == 43) {
				_burlShould = 44;
			}

			_digiName = sound;
		} else if (who == 1) {
			sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
			digi_play(sound, 1, 255, 9);
		}

		_digiMode = KT_PARSE;
		_digiTrigger = 9;
	} else {
		conv_resume();
	}
}

void Room143::talkToVera() {
	conv_load_and_prepare("conv35", 10);

	if (_G(flags)[V000] == 1003) {
		conv_export_value_curr(1, 0);
	} else {
		conv_export_value_curr(0, 0);
	}

	conv_export_value_curr(_G(flags)[V062], 1);
	conv_play_curr();
}

void Room143::talkToBurl() {
	conv_load_and_prepare("conv30", 11, 0);

	if (_burlMode == 42 || _burlMode == 43)
		conv_export_value_curr(1, 0);
	else
		conv_export_value_curr(0, 0);

	conv_export_pointer_curr(&_G(flags)[V053], 3);
	conv_export_pointer_curr(&_G(flags)[V052], 4);
	conv_play_curr();
}

void Room143::loadCheese() {
	_cheese = series_play("143CHES", 0xf00, 0, -1, 600, -1, 100, 35, -5, 0, 0);
}

void Room143::showEmptyPlates() {
	_emptyPlates.terminate();
	_emptyPlates.play("143pl01", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
}

void Room143::playDigi1() {
	if (_digiName) {
		_G(kernel).trigger_mode = _digiMode;
		digi_play(_digiName, 1, 255, _digiTrigger);
		_digiName = nullptr;

		if (_burlMode == 43)
			digi_change_volume(2, 0);
	}
}

void Room143::playDigi2() {
	if (_digiName) {
		_G(kernel).trigger_mode = _digiMode;
		digi_play(_digiName, 1, 255, _digiTrigger);
		_digiName = nullptr;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
