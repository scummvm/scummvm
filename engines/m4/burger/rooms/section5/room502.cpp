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

enum {
	kCHANGE_PURPLE_BORK_ANIMATION = 10
};

enum {
	kBORK_ON_FAN = 5000,
	kBORK_SLIDING_DOWN = 5001,
	kBORK_CLIMBING_STAIRS = 5002
};

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

const seriesStreamBreak Room502::SERIES1[] = {
	{  14, "502Z001", 1, 255, -1, 0, nullptr, 0 },
	{  71, "502Z002", 1, 255, -1, 0, nullptr, 0 },
	{  92, "502Z003", 1, 255, -1, 0, nullptr, 0 },
	{ 124, "502Z004", 1, 255, -1, 0, nullptr, 0 },
	{ 155, "502f001", 1, 255, -1, 0, nullptr, 0 },
	{ 162, "502f002", 1, 255, -1, 0, nullptr, 0 },
	{ 201, nullptr,   0,   0, 20, 0, nullptr, 0 },
	{ 210, "502i001", 1, 255, -1, 0, nullptr, 0 },
	{ 243, "502i002", 1, 255, -1, 0, nullptr, 0 },
	{ 258, "502Z005", 1, 255, -1, 0, nullptr, 0 },
	{ 289, "502i003", 1, 255, -1, 0, nullptr, 0 },
	{ 310, "502Z006", 1, 255, -1, 0, nullptr, 0 },
	{ 334, "502Z007", 1, 255, -1, 0, nullptr, 0 },
	{ 407, "502_005", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room502::SERIES2[] = {
	{ 44, "502_005", 2, 255, -1, 0, nullptr, 0 },
	{ -1, nullptr,   0,   0, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room502::PLAY1[] = {
	{  0,  2, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  3,  6, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{  7, 12, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 13, 18, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 19, 22, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 23, 27, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 28, 32, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 33, 39, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 40, -1, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY2[] = {
	{  0,  0, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  1,  6, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{  7, 12, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 13, 17, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 18, 22, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 24, 29, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 30, 35, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 36, 40, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 41, -1, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY3[] = {
	{  0,  2, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  3,  6, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{  7, 12, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 13, 16, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY4[] = {
	{  0,  0, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  1,  6, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{  7, 12, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 13, 17, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 18, 22, "502_008b", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 24, 27, "502_008a", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY5[] = {
	{ 48, 42, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY6[] = {
	{  0, 14, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 15, 15, "502_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 16, -1, nullptr,   0,   0, 17, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY7[] = {
	{  5, 14, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 15, 15, "502_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 16, -1, nullptr,   0,   0, 17, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY8[] = {
	{ 28, 29, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 30, 35, "502_008b", 1, 100, -1, 0, 0, 0, 0 },
	{ 36, 40, "502_008a", 1, 100, -1, 0, 0, 0, 0 },
	{ 41, -1, "502_008b", 1, 100, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY9[] = {
	{ 0,  4, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 5, -1, "502_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY10[] = {
	{  0, 21, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 22, -1, nullptr, 0, 0, 19, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY11[] = {
	{  0, 15, "502b005a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0, 15, "502b005b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  0, 15, "502b005c", 2, 255, -1, 0, 0, &_state1, 3 },
	{  0, 15, "502b005d", 2, 255, -1, 0, 0, &_state1, 4 },
	{  0, 15, "502b005e", 2, 255, -1, 0, 0, &_state1, 5 },
	{  0, 15, "502b005f", 2, 255, -1, 0, 0, &_state1, 6 },
	{  0, 15, "502b005g", 2, 255, -1, 0, 0, &_state1, 7 },
	{  0, 15, "502b005h", 2, 255, -1, 0, 0, &_state1, 8 },
	{  0, 15, "502b005i", 2, 255, -1, 0, 0, &_state1, 9 },
	{ 16, -1, "502b005a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 16, -1, "502b005b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 16, -1, "502b005c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 16, -1, "502b005d", 2, 255, -1, 0, 0, &_state2, 4 },
	{ 16, -1, "502b005e", 2, 255, -1, 0, 0, &_state2, 5 },
	{ 16, -1, "502b005f", 2, 255, -1, 0, 0, &_state2, 6 },
	{ 16, -1, "502b005g", 2, 255, -1, 0, 0, &_state2, 7 },
	{ 16, -1, "502b005h", 2, 255, -1, 0, 0, &_state2, 8 },
	{ 16, -1, "502b005i", 2, 255, -1, 0, 0, &_state2, 9 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY12[] = {
	{ 0, 2, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 3, 4, "502_002",  2, 255, -1, 0, 3, nullptr,  0 },
	{ 5, 9, "502b008a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 5, 9, "502b008b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 5, 9, "502b008c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 5, 9, "502b008d", 2, 255, -1, 0, 0, &_state1, 4 },
	{ 5, 9, "502b008e", 2, 255, -1, 0, 0, &_state1, 5 },
	{ 9, 9, nullptr,    0,   0, -1, 0, 2, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY13[] = {
	{ 0, 9, "502b007a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 0, 9, "502b007b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 3, 0, nullptr,    0,   0, -1, 2, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY14[] = {
	{ 0, 13, "502b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 0, 13, "502b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 0, 13, "502b006c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 14, 14, nullptr,   0,   0, 16, 0, 0, &_state4, 3 },
	{ 14, 17, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 18, -1, nullptr,   0,   0,  7, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY15[] = {
	{ 0, 4, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 3, 3, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY16[] = {
	{  0, 10, "502_007", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 11, 11, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 12, -1, "500_003", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room502::PLAY17[] = {
	{ 1, -1, "500_008", 2, 255, -1, 4, 0, &_state3, 1 },
	{ 1, -1, "500_009", 2, 255, -1, 4, 0, &_state3, 2 },
	{ 1, -1, "500_010", 2, 255, -1, 4, 0, &_state3, 3 },
	{ 1, -1, "500_011", 2, 255, -1, 4, 0, &_state3, 4 },
	{ 1, -1, "500_012", 2, 255, -1, 4, 0, &_state3, 5 },
	{ 1, -1, "500_013", 2, 255, -1, 4, 0, &_state3, 6 },
	PLAY_BREAK_END
};

int32 Room502::_state1;
int32 Room502::_state2;
int32 Room502::_state3;
int32 Room502::_state4;


Room502::Room502() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
}

void Room502::init() {
	_G(flags)[V194] = 0;
	Section5Room::init();

	pal_cycle_init(124, 127, 12);
	loadSeries1();
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;

	bool skip = false;
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 503:
		ws_demand_location(620, 311, 9);
		ws_walk(304, 308, 0, -1, -1);
		player_set_commands_allowed(true);
		kernel_trigger_dispatch_now(24);
		break;

	case 505:
		ws_demand_location(237, 235, 9);
		ws_hide_walker();
		_G(wilbur_should) = _G(flags)[V196] ? 6 : 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 506:
		ws_demand_location(402, 272, 1);
		_G(wilbur_should) = 12;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
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

	if (_G(flags)[kFireplaceHasFire])
		kernel_trigger_dispatch_now(19);

	_flag1 = true;
	Section5Room::init();
}

void Room502::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, _destTrigger);
		break;

	case 5:
		loadSeries3();
		loadSeries2();
		_series1.show("502bk01", 0x400);
		_series2 = series_show("502spark", 0xc00);

		if (_G(flags)[V185]) {
			kernel_trigger_dispatch_now(20);
			digi_preload_stream_breaks(SERIES2);
			series_stream_with_breaks(SERIES2, "502end", 6, 0x100, 6);
		} else {
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "502intro", 6, 0x100, 6);
		}
		break;

	case 6:
		_series1.terminate();
		terminateMachineAndNull(_series2);
		setup1();
		setup2();
		setup3();

		if (_G(flags)[V185]) {
			digi_unload_stream_breaks(SERIES2);
			_G(wilbur_should) = 10001;
		} else {
			digi_unload_stream_breaks(SERIES1);
			ws_unhide_walker();
			_val2 = 13;
			_G(wilbur_should) = 1;
		}

		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 7:
		ws_unhide_walker();

		switch (_val2) {
		case 13:
			player_set_commands_allowed(true);
			wilbur_speech("502w001");
			break;

		case 14:
			player_set_commands_allowed(true);
			wilbur_speech("502w002");
			break;

		case 15:
			player_set_commands_allowed(true);
			wilbur_speech("502w005");
			break;

		case 16:
			player_set_commands_allowed(true);
			wilbur_speech("502w006");
			break;

		case 17:
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10001;
			wilbur_speech("502w010", kCHANGE_WILBUR_ANIMATION);
			break;

		case 18:
			player_set_commands_allowed(true);
			wilbur_speech("502w025z");
			break;

		case 19:
			_val4 = 34;
			wilbur_speech("500w047", 13);
			break;

		default:
			break;
		}
		break;

	case 8:
		if (_G(flags)[kStairsBorkState] == kBORK_SLIDING_DOWN) {
			player_update_info();

			if (_G(player_info).x < 242 && _G(player_info).y < 265) {
				ws_walk(_G(player_info).x, _G(player_info).y, nullptr, -1);
				player_set_commands_allowed(false);
				kernel_trigger_dispatch_now(15);
			} else {
				kernel_timing_trigger(15, 8);
			}
		}
		break;

	case 9:
		if (!_flag1 && !digi_play_state(1)) {
			_flag1 = true;
			_series1.terminate();
			kernel_trigger_dispatch_now(kCHANGE_PURPLE_BORK_ANIMATION);
		} else {
			kernel_timing_trigger(15, 9);
		}
		break;

	case kCHANGE_PURPLE_BORK_ANIMATION:
		switch (_purpleBorkShould) {
		case 20:
			_flag1 = false;
			_purpleBorkShould = 21;
			kernel_trigger_dispatch_now(kCHANGE_PURPLE_BORK_ANIMATION);
			break;

		case 21:
			_purpleBorkShould = imath_ranged_rand(22, 24);
			kernel_timing_trigger(imath_ranged_rand(240, 360), 9);
			_series1.show("502bk01", 0x400);
			break;

		case 22:
			_state1 = imath_ranged_rand(1, 9);
			_state2 = imath_ranged_rand(1, 9);
			_purpleBorkShould = 20;
			series_play_with_breaks(PLAY11, "502bk01", 0x400, kCHANGE_PURPLE_BORK_ANIMATION, 3);
			break;

		case 23:
			_state1 = imath_ranged_rand(1, 5);
			_purpleBorkShould = 20;
			series_play_with_breaks(PLAY12, "502bk02", 0x400, kCHANGE_PURPLE_BORK_ANIMATION, 3);
			break;

		case 24:
			_purpleBorkShould = 20;
			series_play_with_breaks(PLAY15, "502bk01", 0x400, kCHANGE_PURPLE_BORK_ANIMATION, 3);
			break;

		case 25:
			_series1.terminate();
			_state1 = imath_ranged_rand(1, 2);
			_purpleBorkShould = 20;
			series_play_with_breaks(PLAY13, "502bk03", 0x400, kCHANGE_PURPLE_BORK_ANIMATION, 3);
			break;

		case 26:
			_val2 = 17;
			_state1 = imath_ranged_rand(1, 3);
			_purpleBorkShould = 20;
			series_play_with_breaks(PLAY14, "502bk04", 0x400, kCHANGE_PURPLE_BORK_ANIMATION, 3);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case 11:
		if (_G(flags)[kStairsBorkState] != 5003) {
			player_update_info();

			if ((_G(player_info).x > 242 || _G(player_info).y > 265) &&
					!_flag1 && !digi_play_state(1)) {
				_flag1 = true;
				loadSeries3();
				_val4 = _val7;
				kernel_trigger_dispatch_now(13);
			} else {
				kernel_timing_trigger(15, 11);
			}
		}
		break;

	case 13:
		switch (_val4) {
		case 20:
			digi_stop(2);
			digi_unload("502_006");
			_flag1 = false;
			_val4 = 27;
			kernel_trigger_dispatch_now(13);
			break;

		case 27:
			_G(flags)[kStairsBorkState] = kBORK_ON_FAN;
			_G(flags)[V186] = 0;
			_borkStairs.terminate();
			_val7 = imath_rand_bool(3) ? 29 : 28;

			kernel_timing_trigger(imath_ranged_rand(240, 360), 11);
			break;

		case 28:
			_val8 = 0;
			kernel_trigger_dispatch_now(14);
			_G(flags)[kStairsBorkState] = kBORK_SLIDING_DOWN;
			_G(flags)[V186] = 1;
			kernel_trigger_dispatch_now(8);
			_val4 = 32;
			_borkStairs.play("502bkst", 0xc00, 16, 13, 6, 0, 100, 0, 0, 0, 8);
			break;

		case 29:
			_val8 = 0;
			kernel_trigger_dispatch_now(14);
			_G(flags)[kStairsBorkState] = kBORK_SLIDING_DOWN;
			_G(flags)[V186] = 1;
			kernel_trigger_dispatch_now(8);
			_val4 = 30;
			_borkStairs.play("502bkst", 0xc00, 0, 13, 6, 0, 100, 0, 0, 0, 8);
			break;

		case 30:
			kernel_trigger_dispatch_now(22);
			_val4 = 31;
			_borkStairs.play("502bkst", 0xc00, 0, 13, 6, 0, 100, 0, 0, 9, 18);
			break;

		case 31:
			_val8 = 1;
			kernel_trigger_dispatch_now(14);
			_val4 = 32;
			_borkStairs.play("502bkst", 0xc00, 0, 13, 6, 2, 100, 0, 0, 19, 22);
			break;

		case 32:
			digi_stop(2);
			digi_unload("502_007");
			_borkStairs.terminate();
			_val4 = 33;
			_borkStairs.play("502bkst", 0xc00, 0, 13, 6, 0, 100, 0, 0, 23, 32);
			break;

		case 33:
			_G(flags)[kStairsBorkState] = kBORK_CLIMBING_STAIRS;
			_val4 = 20;
			_borkStairs.play("502bkst", 0xc00, 0, 13, 6, 0, 100, 0, 0, 33, 44);
			digi_preload("502_006");
			digi_play("502_006", 2);
			break;

		case 34:
			if (_flag1) {
				kernel_timing_trigger(30, 13);
			} else {
				_flag1 = true;
				_val8 = 2;
				kernel_trigger_dispatch_now(14);
				_val4 = 35;
				series_play_with_breaks(PLAY16, "502bk09", 0xc00, 13, 3);
			}
			break;

		case 35:
			Section5::flagsTrigger();
			_flag1 = false;
			kernel_trigger_dispatch_now(18);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case 14:
		switch (_val8) {
		case 0:
			digi_play((imath_ranged_rand(1, 2) == 1) ? "502b001a" : "502b001b", 2);
			break;

		case 1:
			digi_play((imath_ranged_rand(1, 2) == 1) ? "502b003a" : "502b003b", 2);
			break;

		case 2:
			digi_play("502b004", 2);
			break;

		default:
			break;
		}
		break;

	case 15:
		_G(flags)[V206] = 5005;
		kernel_trigger_dispatch_now(5015);
		break;

	case 16:
		_G(flags)[V206] = 5006;
		kernel_trigger_dispatch_now(5015);
		break;

	case 17:
		switch (_val5) {
		case 36:
			series_show("502soap2", 0xaff);
			break;

		case 37:
			_val5 = 38;
			series_play("502soap1", 0xaff, 0, 17);
			break;

		case 38:
			_val5 = 36;
			series_play("502soap", 0xaff, 0, 17, 6, 4);
			break;

		default:
			break;
		}
		break;

	case 18:
		series_show("502windo", 0xf00);
		break;

	case 19:
		_G(flags)[kFireplaceHasFire] = 1;
		digi_play_loop("500_002", 3, 125);
		series_play("502fire", 0xc00, 4, -1, 6, -1);
		break;

	case 20:
		inv_give_to_player("GIZMO");
		break;

	case 21:
		if ((_G(flags)[kStairsBorkState] == 5000 || _G(flags)[kStairsBorkState] == 5003) &&
			!_flag1 && !digi_play_state(1)) {
			_flag1 = true;
			kernel_trigger_dispatch_now(22);
		}

		kernel_timing_trigger(imath_ranged_rand(240, 360), 21);
		break;

	case 22:
		terminateMachineAndNull(_series2);
		_state3 = imath_ranged_rand(1, 6);
		series_play_with_breaks(PLAY17, "502spark", 0xc00, 23, 2);
		break;

	case 23:
		if (_G(flags)[kStairsBorkState] == 5000 || _G(flags)[kStairsBorkState] == 5003)
			_flag1 = false;

		_series2 = series_show("502spark", 0xc00);
		break;

	case 24:
		series_play("502smoke", 0x500);
		break;

	case 5002:
		enable_player();
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			kernel_trigger_dispatch_now(7);
			break;

		case 2:
			_G(flags)[V195] = 1;
			break;

		case 3:
			ws_demand_location(237, 235);
			player_set_commands_allowed(false);
			ws_hide_walker();
			_destTrigger = 5009;
			series_play_with_breaks(PLAY1, "502wi02", 0xc01, 1, 3, 5);
			break;

		case 4:
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY2, "502wi03", 0xc01, kCHANGE_WILBUR_ANIMATION, 3, 5);
			break;

		case 5:
			player_set_commands_allowed(false);
			series_load("502wi02");
			series_load("502wi03");
			series_load("502wi05");
			series_load("502bk09");
			ws_demand_location(237, 235, 9);
			ws_hide_walker();
			_G(wilbur_should) = 8;
			series_play_with_breaks(PLAY3, "502wi02", 0xc01, kCHANGE_WILBUR_ANIMATION, 3, 5);
			break;

		case 6:
			_G(wilbur_should) = 7;
			series_play_with_breaks(PLAY4, "502wi03", 0xc01, kCHANGE_WILBUR_ANIMATION, 3, 5);
			break;

		case 7:
			_G(wilbur_should) = 8;
			series_play_with_breaks(PLAY5, "502wi05", 0xc01, kCHANGE_WILBUR_ANIMATION, 2, 5);
			break;

		case 8:
			_val5 = 37;
			_G(wilbur_should) = 9;

			if (_G(flags)[V196]) {
				series_play_with_breaks(PLAY7, "502wi05", 0xc01, kCHANGE_WILBUR_ANIMATION, 2);
			} else {
				series_play_with_breaks(PLAY6, "502wi05", 0xc01, kCHANGE_WILBUR_ANIMATION, 2);
			}

			_G(flags)[V196] = 0;
			break;

		case 9:
			if (_G(flags)[kStairsBorkState] == 5003) {
				_G(wilbur_should) = 10001;
			} else {
				_G(flags)[kStairsBorkState] = 5003;
				_val2 = 19;
				_G(wilbur_should) = 1;
			}

			inv_move_object("SOAPY WATER", NOWHERE);
			inv_give_to_player("BOTTLE");
			series_play_with_breaks(PLAY8, "502wi03", 0xc01, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 10:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 11;
			series_play_with_breaks(PLAY9, "502wi06", 0x801, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 11:
			inv_give_to_player("KINDLING");
			_G(wilbur_should) = 10001;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 12:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_val2 = 18;
			_G(wilbur_should) = 1;
			series_play_with_breaks(PLAY10, "502wi07", 0xbff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room502::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("KITCHEN") && !player_said_any("LOOK AT", "GEAR"))
		player_set_facing_hotspot();
}

void Room502::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool railing = player_said("RAILING") && _G(flags)[kStairsBorkState] == 5003;
	bool takeKindling = player_said("KINDLING ") && player_said("TAKE");
	bool gearKindling = player_said("KINDLING ") && player_said("GEAR");
	bool fireplace = player_said("FIREPLACE") && _G(flags)[kFireplaceHasFire] != 0;

	if (player_said("LOOK AT", "FRONT DOOR") && _G(flags)[V195]) {
		wilbur_speech("502w004");
	} else if (player_said("LOOK AT STAIRS") && _G(flags)[kStairsBorkState] == 5003) {
		wilbur_speech("502w008");
	} else if (railing && player_said("LOOK AT")) {
		wilbur_speech("502w015");
	} else if (railing && player_said("GEAR")) {
		wilbur_speech("502w017");
	} else if (takeKindling && inv_player_has("kindling")) {
		wilbur_speech("502w019");
	} else if (takeKindling && _G(flags)[kFireplaceHasFire]) {
		wilbur_speech("502w020");
	} else if (gearKindling && inv_player_has("KINDLING")) {
		wilbur_speech("502w022");
	} else if (gearKindling && _G(flags)[kFireplaceHasFire]) {
		wilbur_speech("502w020");
	} else if (player_said("KINDLING") && player_said("FIREPLACE")) {
		wilbur_speech(_G(flags)[kFireplaceHasFire] ? "500w063" : "500w062");
	} else if (player_said("KINDLING") && player_said("WIRES")) {
		wilbur_speech("500w065");
	} else if (fireplace && player_said("LOOK AT")) {
		wilbur_speech("502w024");
	} else if (fireplace && player_said("GEAR")) {
		wilbur_speech("502w025");
	} else if (fireplace && player_said("RUBBER DUCK")) {
		wilbur_speech("502w030");
	} else if (player_said("GEAR", "WINDOW") && _G(flags)[kStairsBorkState] == 5003) {
		wilbur_speech("502w026");
	} else if ((player_said("LOOK AT") || player_said("GEAR")) &&
			player_said("PHONE JACK") && _G(flags)[V197] != 0) {
		wilbur_speech("502w033");
	} else if (player_said("LOOK AT", "ROCKER") && _G(flags)[V197] != 0) {
		wilbur_speech("502w037");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "FRONT DOOR")) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("KITCHEN") && player_said_any("LOOK AT", "GEAR")) {
		_destTrigger = 5007;
		kernel_trigger_dispatch_now(1);
	} else if (player_said("GEAR", "STAIRS")) {
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BORK ") && player_said("LOOK AT") &&
			!player_said_any("GIZMO", "ROLLING PIN", "DIRTY SOCK", "SOAPY WATER", "RUBBER GLOVES") &&
			!player_said("LAXATIVE")) {
		player_set_commands_allowed(false);
		_purpleBorkShould = 26;
		++_state4;
	} else if (player_said("TAKE", "KINDLING ")) {
		if (!_G(flags)[kFireplaceHasFire] && !inv_player_has("KINDLING")) {
			_G(wilbur_should) = 10;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("SOAPY WATER", "RAILING")) {
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BURNING KINDLING", "FIREPLACE")) {
		_G(wilbur_should) = 12;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
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
			_purpleBorkShould = 25;
			kernel_timing_trigger(120, 10);

		} else {
			_purpleBorkShould = 20;
			kernel_trigger_dispatch_now(kCHANGE_PURPLE_BORK_ANIMATION);
		}
	}
}

void Room502::setup2() {
	if (_G(flags)[kStairsBorkState] == 5003) {
		_val5 = 36;
		kernel_trigger_dispatch_now(17);
		kernel_trigger_dispatch_now(18);
	} else {
		loadSeries3();
		_borkTable = BORK_TABLE;
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
