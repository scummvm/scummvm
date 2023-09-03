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

#include "m4/burger/rooms/section6/room602.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const Room602::GerbilPoint Room602::GERBIL[] = {
	{ -99, -99 }, { -99, -99 }, { -99, -99 }, { -99, -99 }, { -99, -99 }, { -99, -99 },
	{ 344, 186 }, { 344, 186 }, { 344, 186 }, { 344, 186 }, { 376, 223 }, { 376, 223 },
	{ 342, 201 }, { 392, 228 }, { 392, 228 }, { 349, 231 }, { 392, 228 }, { 392, 228 },
	{ 349, 231 }, { 349, 231 }, { 349, 231 }, { 349, 231 }, { 349, 231 }, { 349, 231 },
	{ 422, 227 }, { 322, 226 }, { 322, 220 }, { 228, 216 }, { 430, 232 }, { 430, 232 },
	{ 183, 230 }, { 430, 232 }, { 430, 232 }, { 179, 233 }, { 426, 231 }, { 426, 231 },
	{ 176, 237 }, { 426, 231 }, { 329, 169 }, { 176, 237 }, { 426, 231 }, { 343, 193 },
	{ 176, 237 }, { 426, 231 }, { 344, 212 }, { 176, 237 }, { 426, 231 }, { 350, 225 },
	{ 176, 237 }, { 426, 231 }, { 350, 225 }, { 176, 237 }, { 426, 231 }, { 344, 227 },
	{ 176, 237 }, { 426, 231 }, { 337, 226 }, { 176, 237 }, { 426, 231 }, { 337, 226 },
	{ 176, 237 }, { 426, 231 }, { 334, 222 }, { 176, 237 }, { 426, 231 }, { 329, 221 },
	{ 176, 237 }, { 426, 231 }, { 324, 219 }, { 176, 237 }, { 426, 231 }, { 320, 218 },
	{ 176, 237 }, { 426, 231 }, { 316, 217 }, { 176, 237 }, { 426, 231 }, { 316, 217 },
	{ 176, 237 }, { 426, 231 }, { 316, 217 }, { 176, 237 }, { 426, 231 }, { 316, 217 },
	{ 176, 237 }, { 426, 231 }, { 316, 217 }, { 176, 237 }, { 426, 231 }, { 316, 217 },
	{ 176, 237 }, { 426, 231 }, { 316, 217 }, { 176, 237 }, { 426, 231 }, { 316, 217 }
};

static const char *SAID[][4] = {
	{ "KIBBLE TRAY",    "602w006", "602w007", "602w008" },
	{ "KIBBLE ",        "602w009", nullptr,   nullptr   },
	{ "EXERCISE WHEEL", nullptr,   "602w014", nullptr   },
	{ "GENERATOR",      nullptr,   "602w023", "602w023" },
	{ "MAGNET",         nullptr,   "602w023", "602w023" },
	{ "DOOR",           nullptr,   "602w008", nullptr   },
	{ "PEANUT",         "602w034", "602w035", "602w036" },
	{ "TUBE",           "602w037", "602w008", nullptr   },
	{ "TUBE ",          "602w038", "602w008", "602w039" },
	{ "BARS",           "602w040", "602w008", "602w008" },
	{ "FLOOR",          nullptr,   "602w008", "602w008" },
	{ "FLOOR  ",        nullptr,   "602w008", "602w008" },
	{ "GERBIL HEAD",    "612w043", "612w044", "602w023" },
	{ "GERBIL HEAD ",   "612w045", "612w046", "612w047" },
	{ "GERBIL PIECES",  nullptr,   "612w050", "612w050" },
	{ "MOTOR",          "612w051", "612w052", nullptr   },
	{ "MOTOR ",         "612w051", "612w052", nullptr   },
	{ "PAW",            nullptr,   "612w060", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesStreamBreak SERIES1[] = {
	{  0, nullptr, 1, 0,    -1, 2048, nullptr, 0 },
	{ 13, nullptr, 0, 0,     2,    0, nullptr, 0 },
	{ 16, nullptr, 0, 0,    14,    0, &Flags::_flags[260], 0 },
	{ 16, nullptr, 0, 0, 10016,    0, &Flags::_flags[260], 1 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{  0, nullptr,   1,   0,    -1, 2048, nullptr, 0 },
	{ 35, nullptr,   0,   0,    11,    0, nullptr, 0 },
	{ 46, "602w018", 1, 255,    -1,    0, nullptr, 0 },
	{ 49, nullptr,   0,   0, 10016,    0, nullptr, 0 },
	{ 55, "602w019", 1, 255,    -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room602::PLAY1[] = {
	{ 0,  0, "602_002", 2, 255, -1, 2048, 0, nullptr, 0 },
	{ 1, -1, "602w001", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY2[] = {
	{  0, 10, "602_001", 2, 255, -1, 2048, 0, nullptr, 0 },
	{ 11, -1, "602w002", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY3[] = {
	{ 0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 4,  5, "600w015", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 6, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY4[] = {
	{  0, 15, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 16, -1, "600_013", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY5[] = {
	{  0, 15, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 16, -1, "600_013", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY6[] = {
	{  0, 19, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 20, -1, "600_013", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY7[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "602_010", 2, 255, -1,    0, 0, nullptr, 0 },
};

const seriesPlayBreak Room602::PLAY8[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "600_014", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY9[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY10[] = {
	{ 0, 67, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY11[] = {
	{ 68, 72, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY12[] = {
	{  0,  3, "600_013", 2, 255, -1, 0, 0, nullptr, 0 },
	{  4,  8, "600_013", 2, 205, -1, 0, 0, nullptr, 0 },
	{  9, 11, "600_013", 2, 175, -1, 0, 0, nullptr, 0 },
	{  0,  4, "600_013", 2, 215, -1, 0, 0, nullptr, 0 },
	{  5,  9, "600_013", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 10, 11, "600_013", 2, 175, -1, 0, 0, nullptr, 0 },
	{  0,  1, "600_013", 2, 225, -1, 0, 0, nullptr, 0 },
	{  2,  7, "600_013", 2, 245, -1, 0, 0, nullptr, 0 },
	{  8, 11, "600_013", 2, 205, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY13[] = {
	{  0, 12, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 13, 15, "602w030", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 13, 15, nullptr,   0,   0, -1,    0, 2, nullptr, 0 },
	{ 16, 21, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 22, -1, "602_008", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY14[] = {
	{  0, 12, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 13, 15, "602w032", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 13, 15, nullptr,   0,   0, -1,    0, 2, nullptr, 0 },
	{ 16, 21, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 22, -1, "602_008", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY15[] = {
	{ 0,  6, nullptr,   1,   0, -1, 2048, 0, 0, 0 },
	{ 7, -1, nullptr,   0,   0, -1,    0, 0, &Flags::_flags[V256], 1 },
	{ 7, -1, nullptr,   0,   0, -1,    0, 0, &Flags::_flags[V256], 2 },
	{ 7, -1, nullptr,   0,   0, -1,    0, 0, &Flags::_flags[V256], 3 },
	{ 7, -1, "602_009", 2, 255, -1,    0, 0, &Flags::_flags[V256], 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY16[] = {
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_test1, 0 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_test1, 1 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_test1, 2 },
	{ 7, -1, "600w011a", 1, 255, -1,    0, 0, &_test1, 0 },
	{ 7, -1, "600w011b", 1, 255, -1,    0, 0, &_test1, 1 },
	{ 7, -1, "600w011c", 1, 255, -1,    0, 0, &_test1, 2 },
	{ 0, -1, "600w011d", 1, 255, -1,    0, 0, &_test1, 3 },
	{ 0, -1, "600w011e", 1, 255, -1,    0, 0, &_test1, 4 },
	{ 0, -1, "600w011f", 1, 255, -1,    0, 0, &_test1, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY17[] = {
	{ 0, 7, "600w011a", 1, 255, -1, 0, 0, &_test1, 0 },
	{ 0, 7, "600w011b", 1, 255, -1, 0, 0, &_test1, 1 },
	{ 0, 7, "600w011c", 1, 255, -1, 0, 0, &_test1, 2 },
	{ 0, 7, "600w011d", 1, 255, -1, 0, 0, &_test1, 3 },
	{ 0, 7, "600w011e", 1, 255, -1, 0, 0, &_test1, 4 },
	{ 0, 7, "600w011f", 1, 255, -1, 0, 0, &_test1, 5 },
	{ 6, 1, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{ 0, 7, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY18[] = {
	{  0, 13, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 14, -1, "600_012", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY19[] = {
	{ 0,  6, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 7, 24, "612w053", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY20[] = {
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, nullptr, 0  },
	{ 7, -1, "612w054a", 1, 255, -1,    0, 0, &_test2, 0  },
	{ 7, -1, "612w054b", 1, 255, -1,    0, 0, &_test2, 1  },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY21[] = {
	{ 0, 6, 0, 0, 0, -1, 0, 0, 0, 0  },
	{ 7, 27, "612w054y", 1, 255, -1, 0, 0, 0, 0  },
	PLAY_BREAK_END
};

const seriesPlayBreak Room602::PLAY22[] = {
	{  0,  6, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  7, 18, "612w054y", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 19, -1, nullptr,    0,   0,  3, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

long Room602::_test1;
long Room602::_test2;

Room602::Room602() : Section6Room() {
	_gerbilTable = GERBIL;
	_test1 = _test2 = 0;
}

void Room602::init() {
	player_set_commands_allowed(false);
	_G(flags)[V256] = 0;
	_series1 = 0;

	if (_G(game).room_id == 612) {
		digi_preload("612_001a");

		switch (_G(flags)[V277]) {
		case 6001:
			_val1 = 55;
			break;

		case 6002:
			_val1 = _G(flags)[V278] ? 56 : 55;
			break;

		case 6003:
			if (_G(flags)[V278]) {
				digi_preload("602_005");
				_val1 = 58;
			} else {
				_val1 = 57;
			}
			break;

		default:
			break;
		}

		if (_G(flags)[V278]) {
			term_message("Adding the pushed walk code.");
			_walk1 = intr_add_no_walk_rect(322, 304, 472, 329, 312, 320);
			hotspot_set_active("PAW", false);
			hotspot_set_active("MOTOR ", true);
			hotspot_set_active("MOTOR", false);
			hotspot_set_active("FLOOR  ", false);

		} else {
			term_message("Adding the non-pushed walk code.");
			term_message("x1 = %d, y1 = %d, x2 = %d, y2 = %d", 208, 304, 369, 329);
			_walk1 = intr_add_no_walk_rect(208, 304, 369, 329, 197, 322);
			hotspot_set_active("PAW", true);
			hotspot_set_active("MOTOR", true);
			hotspot_set_active("MOTOR ", false);
			hotspot_set_active("FLOOR  ", true);
		}

		kernel_trigger_dispatch_now(12);

		if (_G(game).previous_room != 609)
			kernel_trigger_dispatch_now(1);

		_val2 = 48;
	} else {
		_val2 = 0;
	}

	_G(flags)[V264] = 0;
	if (_G(flags)[V255] == 1)
		series_show("602spill", 0xf00);

	if (_G(flags)[V255] && _G(flags)[V278])
		_G(kernel).call_daemon_every_loop = true;

	if (_G(flags)[V277] == 6003 && _G(flags)[V278] == 1) {
		_series2 = series_play("612wheel", 0x5ff);

	} else if (_G(game).room_id == 502) {
		_series2 = series_show("602wheel", 0x6ff);

	} else {
		_series2 = series_show("612wheel", 0x6ff);
	}

	_series3 = series_show("602door", 0xf00, 1, -1, -1, 0, 100,
		_G(flags)[V257] / 21, _G(flags)[V257]);

	if (_G(flags)[V256] == 1) {
		_val3 = 63;
		kernel_trigger_dispatch_now(2);
	}

	if (_G(flags)[V256] == 3) {
		_val3 = 64;
		kernel_trigger_dispatch_now(2);
	}

	if (_G(flags)[V256]) {
		hotspot_set_active("DOOR", false);
		hotspot_set_active("EXIT", true);

	} else {
		hotspot_set_active("DOOR", true);
		hotspot_set_active("EXIT", false);
	}

	if (_G(flags)[V256] == 10028) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	if (_G(flags)[V243] == 6006) {
		_series4 = series_load("602mg01");
		_series5 = series_load("602mg01s");
		_series6 = series_load("602mg02");
		_series7 = series_load("602mg02s");
		Section6::_state3 = 6001;
		kernel_timing_trigger(60, 6011);
	}

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		if (_G(flags)[V263]) {
			_G(wilbur_should) = 11;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 603:
		ws_hide_walker();
		if (_G(roomVal7) == 1) {
			_G(wilbur_should) = 2;
			kernel_timing_trigger(30, gCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 3;
			kernel_timing_trigger(60, gCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 604:
		ws_hide_walker();
		_G(wilbur_should) = 3;
		kernel_timing_trigger(60, gCHANGE_WILBUR_ANIMATION);
		break;

	case 609:
		ws_demand_location(548, 355, 9);
		_G(wilbur_should) = 10002;

		hotspot_set_active("PAW", true);
		hotspot_set_active("MOTOR", true);
		hotspot_set_active("MOTOR ", false);
		hotspot_set_active("FLOOR  ", true);

		_series8 = series_play("612mot02", 0x700, 0, -1, 0, -1);
		_val1 = 53;
		kernel_trigger_dispatch_now(1);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(548, 355, 9);

		if (_G(game).room_id == 612) {
			_G(flags)[V277] = 6001;
			_G(flags)[V243] = 6007;
			_G(flags)[V244] = 6004;
			_G(flags)[V245] = 10031;
			_G(flags)[ROOM101_FLAG8] = 1;
			_val1 = 53;
			kernel_trigger_dispatch_now(1);
		}
		break;
	}

	if (_G(flags)[V243] == 6006)
		Section6::_state4 = 5;
	if (_G(flags)[V243] == 6007)
		Section6::_state4 = 9;
	else if (_G(flags)[V243] == 6000)
		Section6::_state4 = 2;

	kernel_trigger_dispatch_now(6014);
}

void Room602::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		switch (_val1) {
		case 53:
			player_set_commands_allowed(false);
			digi_preload("612_001b");
			_series1 = series_load("612mot01");
			series_load("612mot02");
			series_play("602mg03", 1);
			wilbur_speech("602w004");
			terminateMachineAndNull(_series8);
			_val1 = 54;
			_series8 = series_play("612mot01", 0x600, 0, 1, 0, 8, 100, 0, 0, 0, 7);
			break;

		case 54:
			_G(flags)[V249] = 1;
			digi_play("612_001b", 3, 255, 12, 612);
			_val1 = 55;
			wilbur_speech("602w005");
			_series8 = series_play("612mot01", 0x400, 0, 1, 0, 0, 100, 0, 0, 8, 18);
			_G(wilbur_should) = 10001;
			kernel_timing_trigger(30, gCHANGE_WILBUR_ANIMATION);
			break;

		case 55:
			_G(flags)[V278] = 0;
			if (_series1)
				series_unload(_series1);

			_series8 = series_play("612mot02", 0x600, 0, -1, 0);
			hotspot_set_active("PAW", true);
			hotspot_set_active("MOTOR", true);
			hotspot_set_active("MOTOR ", false);
			hotspot_set_active("FOOR  ", true);
			break;

		case 56:
			_G(flags)[V279] = 2;
			terminateMachineAndNull(_series8);
			_G(flags)[V278] = 1;
			_series8 = series_play("612mot02", 0x700, 0, -1, 0, -1, 100, 114, -2);

			hotspot_set_active("PAW", false);
			hotspot_set_active("MOTOR", false);
			hotspot_set_active("MOTOR ", true);
			hotspot_set_active("FLOOR  ", false);
			break;

		case 57:
			_G(flags)[V279] = 2;
			_series8 = series_play("612mot03", 0x600, 0, -1, 0);
			inv_move_object("PANTYHOSE", NOWHERE);
			_G(flags)[V278] = 0;

			hotspot_set_active("PAW", true);
			hotspot_set_active("MOTOR", true);
			hotspot_set_active("MOTOR ", false);
			hotspot_set_active("FLOOR  ", true);
			break;

		case 58:
			digi_play_loop("602_004", 3, 255, 6, 602);
			term_message("*** Play wheel... 1");
			_G(flags)[V279] = 2;
			term_message("Run with push.");

			_series8 = series_play("612mot03", 0x600, 0, -1, 0, -1, 100, 114, -2);
			inv_move_object("PANTYHOSE", 999);
			_G(flags)[V278] = 1;

			hotspot_set_active("PAW", false);
			hotspot_set_active("MOTOR", false);
			hotspot_set_active("MOTOR ", true);
			hotspot_set_active("FLOOR  ", false);
			break;

		case 59:
			_G(flags)[V260] = 1;
			_G(flags)[V279] = 2;
			_val1 = 60;
			// TODO
			if (_G(flags)[V278]) {
//				series_play_with_breaks(PLAY, "612wil7", 0x700, 1, 3, 6, 100, 114, -2);
	
			} else {

			}
			break;
		}
		break;

	case 3:
		_G(flags)[V258] = 40;
		_G(flags)[V256] = 1;
		_val3 = 63;
		kernel_trigger_dispatch_now(2);
		terminateMachineAndNull(_series2);

		_series2 = series_play(_G(game).room_id == 602 ? "602wheel" : "612wheel",
			0x5ff, 0, -1, 0);
		hotspot_set_active("DOOR", false);
		hotspot_set_active("EXIT", true);
		break;

	case 5:
		disable_player_commands_and_fade_init(6009);
		break;

	case 6:
		digi_play_loop("602_005", 2, 255, -1, 602);
		digi_unload("602_004");
		break;

	case 7:
		inv_give_to_player("KIBBLE");
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	case 8:
		inv_move_object("KIBBLE", NOWHERE);
		break;

	case 9:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(10);
		break;

	case 10:
		series_show("602spill", 0xf00);
		kernel_trigger_dispatch_now(8);
		break;

	case 11:
		_val3 = 64;
		_G(flags)[V256] = 3;
		kernel_timing_trigger(1, 2);
		break;

	case 12:
		digi_play_loop("612_001a", 3, 255, -1, 612);
		break;

	case 13:
		_G(flags)[V263] = 1;
		_series2 = series_play(_G(game).room_id == 602 ? "602wi07b" : "612wi07b", 0x6ff);
		player_set_commands_allowed(true);
		term_message("Ready to keep running...");
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room602::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V263] == 1) {
		_G(wilbur_should) = 13;
		_G(player).command_ready = false;
		player_set_commands_allowed(false);
		intr_cancel_sentence();
		hotspot_set_active("DOOR", true);
		hotspot_set_active("EXIT", false);

	} else if (_G(flags)[V243] == 6006 && (player_said("GEAR", "TUBE") || player_said("CLIMB IN"))) {
		wilbur_speech("600w003");
		intr_cancel_sentence();

	} else if (player_said("KIBBLE") && (player_said("FLOOR ") || player_said("FLOOR  ") || player_said("FLOOR"))) {
		if (_G(flags)[V278] == 0) {
			if (_G(flags)[V255] == 0) {
				_G(wilbur_should) = 22;
				player_hotspot_walk_override(423, 303, 9, gCHANGE_WILBUR_ANIMATION);
			} else {
				wilbur_speech("600w008z");
			}
		}
	} else if (player_said("RAY GUN", "KIBBLE TRAY")) {
		_G(wilbur_should) = 6;
		if (_G(game).room_id == 602) {
			player_hotspot_walk_override(172, 325, 9, gCHANGE_WILBUR_ANIMATION);
		} else {
			player_hotspot_walk_override(151, 315, 9, gCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("RAY GUN", "KIBBLE ")) {
		_G(wilbur_should) = 7;
		player_hotspot_walk_override(151, 315, 9, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "EXERCISE WHEEL") && !_G(flags)[V278]) {
		_G(wilbur_should) = 8;
		player_hotspot_walk_override(409, 359, 2, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "DOOR")) {
		_G(wilbur_should) = 9;
		player_hotspot_walk_override(331, 303, 10, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "GERBILS")) {
		_G(wilbur_should) = 10;
		player_hotspot_walk_override(315, 317, 10, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "DOOR")) {
		_G(wilbur_should) = 47;
		player_hotspot_walk_override(257, 290, 10, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "MOTOR") || player_said("GEAR", "MOTOR ")) {
		if (_G(flags)[V278] == 0) {
			if (_G(flags)[V277] == 6001) {
				_G(wilbur_should) = 25;
				player_hotspot_walk_override(200, 321, 3, gCHANGE_WILBUR_ANIMATION);
			}
			if (_G(flags)[V277] == 6002 || _G(flags)[V277] == 6003) {
				if (_G(flags)[V255] == 1) {
					_G(wilbur_should) = 39;
					player_hotspot_walk_override(200, 21, 3, gCHANGE_WILBUR_ANIMATION);
				} else {
					_G(wilbur_should) = 26;
					player_hotspot_walk_override(200, 321, 3, gCHANGE_WILBUR_ANIMATION);
				}
			}
		} else {
			_G(wilbur_should) = 26;
			player_hotspot_walk_override(314, 319, 3, gCHANGE_WILBUR_ANIMATION);
		}
	} else if ((player_said("GEAR", "MOTOR ") || player_said("PANTYHOSE", "MOTOR ")) &&
			_G(flags)[V277] != 6002) {
		_G(wilbur_should) = 37;
		if (_G(flags)[V278]) {
			player_hotspot_walk_override(313, 319, 3, gCHANGE_WILBUR_ANIMATION);
		} else {
			player_hotspot_walk_override(200, 321, 3, gCHANGE_WILBUR_ANIMATION);
		}
	} else {
		return;
	}
}

void Room602::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("RAY GUN") && (player_said("MOTOR") ||
			player_said("MOTOR ") || player_said("PAW") || player_said("GENERATOR") ||
			player_said("MAGNET"))) {
		_G(wilbur_should) = 43;
		intr_cancel_sentence();
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "PEANUT")) {
		wilbur_speech("602w047");
		intr_cancel_sentence();
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "TUBE") || player_said("CLIMB IN", "TUBE")) {
		_G(flags)[V246] = 1;
		Section6::_state2 = 2;
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("EXIT")) {
		_G(wilbur_should) = 42;
		ws_turn_to_face(5, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("PANTYHOSE", "MOTOR") || player_said("PANTYHOSE", "MOTOR ")) {
		if (_G(flags)[V277] == 6001) {
			_G(wilbur_should) = 25;
			if (_G(flags)[V278]) {
				player_hotspot_walk_override(314, 321, 3, gCHANGE_WILBUR_ANIMATION);
			} else {
				player_hotspot_walk_override(200, 321, 3, gCHANGE_WILBUR_ANIMATION);
			}
		}

		if (_G(flags)[V280]) {
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		} else {
			wilbur_speech("612w064", gCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("GEAR", "EXERCISE WHEEL")) {
		_G(wilbur_should) = 11;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "DOOR")) {
		_G(wilbur_should) = 47;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("KIBBLE ", "TAKE")) {
		_G(wilbur_should) = 18;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("LOOK AT", "EXERCISE WHEEL")) {
		if (_G(flags)[V278] == 1 && _G(flags)[V277] == 6003) {
			wilbur_speech("612w062");
		} else {
			wilbur_speech("602w013");
		}
	} else if (player_said("LOOK AT", "MAGNET")) {
		wilbur_speech(_G(flags)[V260] ? "602w025" : "602w024");

	} else if (player_said("LOOK AT", "FLOOR") || player_said("LOOK AT", "FLOOR  ")) {
		wilbur_speech(_G(game).room_id == 612 ? "602w042" : "602w041");

	} else if (player_said("LOOK AT", "GENERATOR")) {
		wilbur_speech(_G(flags)[V260] ? "602w022" : "602w021");

	} else if (player_said("LOOK AT", "GERBIL PIECES") || player_said("LOOK AT", "GERBIL HAND")) {
		wilbur_speech(_G(flags)[V281] ? "612w049" : "612w048");

	} else if (player_said("LOOK AT", "DOOR")) {
		if (_G(flags)[V261]) {
			wilbur_speech(_G(flags)[V260] ? "602w029" : "602w028");
		} else {
			wilbur_speech(_G(flags)[V260] ? "602w027" : "602w026");
		}
	} else if (player_said("LOOK AT", "PAW")) {
		wilbur_speech(_G(flags)[V277] == 6003 ? "612w059" : "612w058");

	} else if (player_said("PAW", "GEAR") && _G(flags)[V277] != 6003) {
		wilbur_speech("612w061");

	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
