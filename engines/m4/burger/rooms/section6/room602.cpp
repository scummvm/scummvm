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

enum {
	kCHANGE_MOTOR_ANIMATION = 1,
	kCHANGE_DOOR_ANIMATION = 2
};

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
	PLAY_BREAK_END
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
	{ 7, -1, nullptr,   0,   0, -1,    0, 0, &Flags::_flags[kGerbilCageDoor], 1 },
	{ 7, -1, nullptr,   0,   0, -1,    0, 0, &Flags::_flags[kGerbilCageDoor], 2 },
	{ 7, -1, nullptr,   0,   0, -1,    0, 0, &Flags::_flags[kGerbilCageDoor], 3 },
	{ 7, -1, "602_009", 2, 255, -1,    0, 0, &Flags::_flags[kGerbilCageDoor], 0 },
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

int32 Room602::_test1;
int32 Room602::_test2;

Room602::Room602() : Section6Room() {
	_gerbilTable = GERBIL;
	_test1 = _test2 = 0;
}

void Room602::init() {
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;
	_series1 = 0;

	if (_G(game).room_id == 612) {
		digi_preload("612_001a");

		switch (_G(flags)[V277]) {
		case 6001:
			_motorShould = 55;
			break;

		case 6002:
			_motorShould = _G(flags)[V278] ? 56 : 55;
			break;

		case 6003:
			if (_G(flags)[V278]) {
				digi_preload("602_005");
				_motorShould = 58;
			} else {
				_motorShould = 57;
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
			kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);

		_kibbleOffset = 48;
	} else {
		_kibbleOffset = 0;
	}

	_G(flags)[kStandingOnKibble] = 0;
	if (_G(flags)[V255] == 1)
		series_show("602spill", 0xf00);

	if (_G(flags)[V255] && !_G(flags)[V278])
		_G(kernel).call_daemon_every_loop = true;

	if (_G(flags)[V277] == 6003 && _G(flags)[V278] == 1) {
		_mouseWheel = series_play("612wheel", 0x5ff, 0, -1, 0, -1);
		_series10 = series_play("612magnt", 0x600, 1, -1, 1, -1);
	} else if (_G(game).room_id == 602) {
		_mouseWheel = series_show("602wheel", 0x6ff, 0, -1, -1, 0);

	} else {
		_mouseWheel = series_show("612wheel", 0x6ff, 0, -1, -1, 0);
	}

	_series3 = series_show("602door", 0xf00, 1, -1, -1, 0, 100,
		-_G(flags)[V257] / 21, _G(flags)[V257]);

	if (_G(flags)[kGerbilCageDoor] == 1) {
		_doorShould = 63;
		kernel_trigger_dispatch_now(kCHANGE_DOOR_ANIMATION);
	}

	if (_G(flags)[kGerbilCageDoor] == 3) {
		_doorShould = 64;
		kernel_trigger_dispatch_now(kCHANGE_DOOR_ANIMATION);
	}

	if (_G(flags)[kGerbilCageDoor]) {
		hotspot_set_active("DOOR", false);
		hotspot_set_active("EXIT", true);

	} else {
		hotspot_set_active("DOOR", true);
		hotspot_set_active("EXIT", false);
	}

	if (_G(flags)[V245] == 10028) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	if (_G(flags)[kHampsterState] == 6006) {
		_series4 = series_load("602mg01");
		_series5 = series_load("602mg01s");
		_series6 = series_load("602mg02");
		_series7 = series_load("602mg02s");
		Section6::_gerbilState = 6001;
		kernel_timing_trigger(60, 6011);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		if (_G(flags)[V263]) {
			_G(wilbur_should) = 11;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 603:
		ws_hide_walker();
		if (_G(roomVal7) == 1) {
			_G(roomVal7) = 0;
			_G(wilbur_should) = 2;
			kernel_timing_trigger(30, kCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 3;
			kernel_timing_trigger(60, kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 604:
		ws_hide_walker();
		_G(wilbur_should) = 3;
		kernel_timing_trigger(60, kCHANGE_WILBUR_ANIMATION);
		break;

	case 609:
		ws_demand_location(548, 355, 9);
		_G(wilbur_should) = 10002;

		hotspot_set_active("PAW", true);
		hotspot_set_active("MOTOR", true);
		hotspot_set_active("MOTOR ", false);
		hotspot_set_active("FLOOR  ", true);

		_series8 = series_play("612mot02", 0x700, 0, -1, 0, -1);
		_motorShould = 53;
		kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(548, 355, 9);

		if (_G(game).room_id == 612) {
			_G(flags)[V277] = 6001;
			_G(flags)[kHampsterState] = 6007;
			_G(flags)[V244] = 6004;
			_G(flags)[V245] = 10031;
			_G(flags)[V248] = 1;
			_motorShould = 53;
			kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);
		}
		break;
	}

	if (_G(flags)[kHampsterState] == 6006)
		Section6::_state4 = 5;
	if (_G(flags)[kHampsterState] == 6007)
		Section6::_state4 = 9;
	else if (_G(flags)[kHampsterState] == 6000)
		Section6::_state4 = 2;

	kernel_trigger_dispatch_now(6014);
}

void Room602::daemon() {
	switch (_G(kernel).trigger) {
	case kCHANGE_MOTOR_ANIMATION:
		switch (_motorShould) {
		case 53:
			player_set_commands_allowed(false);
			digi_preload("612_001b");
			_series1 = series_load("612mot01");
			series_load("612mot02");
			series_play("602mg03", 1);
			wilbur_speech("602w004");
			terminateMachineAndNull(_series8);
			_motorShould = 54;
			_series8 = series_play("612mot01", 0x600, 0, 1, 0, 8, 100, 0, 0, 0, 7);
			break;

		case 54:
			_G(flags)[V249] = 1;
			digi_play("612_001b", 3, 255, 12, 612);
			_motorShould = 55;
			wilbur_speech("602w005");
			_series8 = series_play("612mot01", 0x400, 0, 1, 0, 0, 100, 0, 0, 8, 18);
			_G(wilbur_should) = 10001;
			kernel_timing_trigger(30, kCHANGE_WILBUR_ANIMATION);
			break;

		case 55:
			_G(flags)[V278] = 0;
			if (_series1)
				series_unload(_series1);

			_series8 = series_play("612mot02", 0x600, 0, -1, 0, -1);
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
			_series8 = series_play("612mot03", 0x600, 0, -1, 0, -1);
			inv_move_object("PANTYHOSE", NOWHERE);
			_G(flags)[V278] = 0;

			hotspot_set_active("PAW", true);
			hotspot_set_active("MOTOR", true);
			hotspot_set_active("MOTOR ", false);
			hotspot_set_active("FLOOR  ", true);
			break;

		case 58:
			digi_play("602_004", 3, 255, 6, 602);
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
			_motorShould = 60;

			if (_G(flags)[V278]) {
				series_play_with_breaks(PLAY11, "612wi17", 0x700, 1, 3, 6, 100, 114, -2);
			} else {
				series_play_with_breaks(PLAY11, "612wi17", 0x6ff, 1, 3, 6, 100, 0, 0);
			}
			break;

		case 60:
			_G(flags)[V280] = 1;
			kernel_trigger_dispatch_now(3);
			_motorShould = 58;
			kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);

			_G(wilbur_should) = (_G(flags)[V280] == 1) ? 10001 : 35;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 61:
			_G(flags)[V260] = 1;
			_G(flags)[V279] = 2;
			_G(wilbur_should) = 10001;
			_motorShould = 58;
			break;

		default:
			break;
		}
		break;

	case kCHANGE_DOOR_ANIMATION:
		switch (_doorShould) {
		case 48:
			_magnetState = 17;
			kernel_trigger_dispatch_now(4);
			break;

		case 62:
			_G(flags)[kGerbilCageDoor] = 1;
			_G(flags)[V258] = 55;
			_G(flags)[V257] = 0;
			_doorShould = 63;
			digi_play("602_004", 2, 255, 6, 602);
			kernel_timing_trigger(1, 2);
			_magnetState = 16;
			kernel_trigger_dispatch_now(4);
			break;

		case 63:
			if (_G(flags)[V258]) {
				terminateMachineAndNull(_series3);
				_series3 = series_show("602door", 0xf00, 0, 2, 6, 0, 100,
					-_G(flags)[V257] / 21, _G(flags)[V257]);
				_G(flags)[V257] -= _G(flags)[V258] / 32;
				_G(flags)[kGerbilCageDoor] = 1;

				if (_G(flags)[V257] + 140 >= 20) {
					_G(flags)[V258] += 3;
				} else {
					_G(flags)[V258] -= 32;
					if (_G(flags)[V258] < 15)
						_G(flags)[V258] = 0;
				}
			} else {
				_G(flags)[kGerbilCageDoor] = 2;
				_doorShould = 48;
				terminateMachineAndNull(_series3);
				_series3 = series_show("602door", 0xf00, 0, -1, -1, 0, 100,
					-_G(flags)[V257] / 21, _G(flags)[V257]);
			}

			_magnetState = 16;
			kernel_trigger_dispatch_now(4);
			break;

		case 64:
			if (_G(flags)[V257] > 0) {
				digi_play("602_007", 2);
				_G(flags)[kGerbilCageDoor] = 0;
				_G(flags)[V257] = 0;
				_doorShould = 48;

				terminateMachineAndNull(_series3);
				_series3 = series_show("602door", 0xf00, 1);

			} else {
				_G(flags)[V257] -= _G(flags)[V258] >> 5;
				_G(flags)[kGerbilCageDoor] = 3;
				_G(flags)[V258] -= 40;
				terminateMachineAndNull(_series3);

				_series3 = series_show("602door", 0xf00, 1, 2, 6, 0, 100,
					-_G(flags)[V257] / 21, _G(flags)[V257]);
			}

			_magnetState = 17;
			kernel_trigger_dispatch_now(4);
			break;

		default:
			break;
		}
		break;

	case 3:
		_G(flags)[V258] = 40;
		_G(flags)[kGerbilCageDoor] = 1;
		_doorShould = 63;
		kernel_trigger_dispatch_now(kCHANGE_DOOR_ANIMATION);
		terminateMachineAndNull(_mouseWheel);

		_mouseWheel = series_play(_G(game).room_id == 602 ? "602wheel" : "612wheel",
			0x5ff, 0, -1, 0, -1);
		hotspot_set_active("DOOR", false);
		hotspot_set_active("EXIT", true);
		break;

	case 4:
		switch (_magnetState) {
		case 16:
			if (!_G(flags)[V265]) {
				if (_G(game).room_id == 602) {
					_series10 = series_play("602magnt", 0x600, 1, -1, 1, -1);
				} else {
					_series10 = series_play("612magnt", 0x600, 1, -1, 1, -1);
				}
			}

			_G(flags)[V265] = 1;
			break;

		case 17:
			if (_G(flags)[V265])
				terminateMachineAndNull(_series10);
			_G(flags)[V265] = 0;
			break;

		default:
			break;
		}
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
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 8:
		inv_move_object("KIBBLE", NOWHERE);
		break;

	case 9:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(10);
		break;

	case 10:
		series_show("602spill", 0xf00);
		kernel_trigger_dispatch_now(8);
		break;

	case 11:
		_doorShould = 64;
		_G(flags)[kGerbilCageDoor] = 3;
		kernel_timing_trigger(1, 2);
		break;

	case 12:
		digi_play_loop("612_001a", 3, 255, -1, 612);
		break;

	case 13:
		_G(flags)[V263] = 1;
		_mouseWheel = series_play(_G(game).room_id == 602 ? "602wi07b" : "612wi07b", 0x6ff, 0, -1, 6, -1);
		player_set_commands_allowed(true);
		term_message("Ready to keep running...");
		break;

	case 14:
		digi_play("602w015", 1, 255, kCHANGE_WILBUR_ANIMATION, 602);
		break;

	case 6011:
		switch (Section6::_gerbilState) {
		case 49:
			series_unload(_series4);
			series_unload(_series5);
			_G(flags)[V266] = 1;
			Section6::_state4 = 6;
			kernel_trigger_dispatch_now(6014);
			Section6::_gerbilState = 50;
			series_play_with_breaks(PLAY12, "602mg02", 0xc00, 6011, 3, 6, 100, 0, 0);
			break;

		case 50:
			if (_G(my_walker)) {
				player_update_info();
				term_message("Wilbur's position: (%d, %d)", _G(player_info).x, _G(player_info).y);

				if (_G(player_info).x < 511 || _G(player_info).y < 322) {
					term_message("Wilbur is gonna die");
					Section6::_gerbilState = 6005;
				} else {
					term_message("Wilbur is safe");
					Section6::_gerbilState = 51;
				}

				_G(kernel).continue_handling_trigger = true;
				kernel_trigger_dispatch_now(6011);
			}
			break;

		case 51:
			_G(flags)[V266] = 0;
			_G(flags)[kHampsterState] = 6007;
			_G(flags)[V277] = 6001;
			_G(flags)[V244] = 6004;
			_G(flags)[V245] = 10031;
			player_set_commands_allowed(false);
			_motorShould = 53;
			kernel_trigger_dispatch_now(6008);
			break;

		case 6001:
			Section6::_gerbilState = 6002;
			_sectionMachine1 = series_play("602mg01", 0xc00, 0, 6011, 8, 0, 100, 0, 0, 0, 67);
			_sectionMachine2 = series_play("602mg01s", 0xc01, 0, -1, 8, 0, 100, 0, 0, 0, 67);
			break;


		case 6002:
			Section6::_state4 = 4;
			kernel_trigger_dispatch_now(6014);
			_sectionMachine1 = series_play("602mg01", 0xc00, 0, 6011, 8, 0, 100, 0, 0, 68, -1);
			_sectionMachine2 = series_play("602mg01s", 0xc01, 0, -1, 8, 0, 100, 0, 0, 68, -1);

			Section6::_gerbilState = (_G(flags)[V245] == 10028) ? 49 : 6004;
			break;

		case 6004:
			series_unload(_series4);
			series_unload(_series5);
			player_set_commands_allowed(false);
			kernel_trigger_dispatch_now(6005);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);
			ws_demand_location(-66, 240, 11);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY3, "602wi03", 0xc00, 6003, 3);
			break;

		case 2:
			ws_demand_location(283, 338, 7);
			ws_hide_walker();
			_G(wilbur_should) = 4;
			series_play_with_breaks(PLAY2, "602wi01", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			_G(flags)[kStandingOnKibble] = 0;
			ws_demand_location(367, 280, 5);
			ws_hide_walker();
			_G(wilbur_should) = 4;
			series_play_with_breaks(PLAY1, "602wi02", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			ws_unhide_walker();
			player_set_commands_allowed(true);

			if (_G(player).been_here_before) {
				_G(wilbur_should) = 10001;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				player_set_commands_allowed(false);
				_G(wilbur_should) = 45;
				ws_turn_to_face(10, kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 6:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 30;

			if (_G(game).room_id == 602) {
				series_play_with_breaks(PLAY4, "602wi05", 0x500, kCHANGE_WILBUR_ANIMATION, 3);
			} else {
				series_play_with_breaks(PLAY5, "602wi19", 0x600, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 7:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 31;
			series_play_with_breaks(PLAY5, "602wi19", 0x600, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 8:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 32;
			series_play_with_breaks(PLAY6, "602wi18", 0x300, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 9:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 33;
			series_play_with_breaks(PLAY7, "602wi21", 0x900, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 10:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY8, "602wi40", 0x600, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 11:
			if (_G(flags)[V278] == 0 || _G(flags)[V277] != 6003) {
				player_set_commands_allowed(false);
				digi_stop(1);

				if (_G(flags)[kHampsterState] == 6006) {
					Walker::unloadSprites();

				} else {
					ws_demand_location(431, 343, 10);
					ws_hide_walker();
				}

				static const char *NAMES[14] = {
					"602_004", "602_005", "602_006", "602w015", "602w017a",
					"602w017b", "602w017c", "602w017d", "602w017e", "602w017f",
					"602w016a", "602w016b", "602w016c", "602w016d"
				};
				for (int i = 0; i < 14; ++i)
					digi_preload(NAMES[i]);

				_G(flags)[V262] = 0;
				hotspot_set_active("DOOR", false);
				hotspot_set_active("EXIT", true);

				digi_preload_stream_breaks(SERIES1);
				digi_preload_stream_breaks(SERIES2);
				terminateMachineAndNull(_mouseWheel);

				_sectionSeries2 = series_load("602wi7as");
				_sectionSeries3 = series_load("602wi7bs");
				_sectionSeries1 = series_load((_G(game).room_id == 612) ? "612wi07b" : "602wi07b");
				_G(wilbur_should) = 12;

				if (_G(flags)[V263]) {
					kernel_trigger_dispatch_now(13);
					kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

				} else {
					_series9 = series_play("602wi7as", 6, 0x700, -1);
					_doorShould = 62;
					series_stream_with_breaks(SERIES1,
						(_G(game).room_id == 602) ? "602wi07a" : "612wi07a",
						6, 0x6ff, 13);
				}
			} else {
				wilbur_speech("612w063");
			}
			break;

		case 12:
			if (_G(flags)[V262] == 4) {
				digi_play(Common::String::format("602w017%c", 'a' + imath_ranged_rand(0, 6)).c_str(),
					1, 255, kCHANGE_WILBUR_ANIMATION, 602);
			} else {
				digi_play(Common::String::format("602w016%c", 'a' + imath_ranged_rand(0, 3)).c_str(),
					1, 255, kCHANGE_WILBUR_ANIMATION, 602);
			}
			break;

		case 13:
			digi_play("602_006", 2, 255, -1, 602);
			digi_unload("602_005");
			player_set_commands_allowed(false);
			_G(flags)[V263] = 0;

			terminateMachineAndNull(_mouseWheel);
			_G(wilbur_should) = 14;

			if (_G(game).room_id == 602) {
				series_stream_with_breaks(SERIES2, "602wi07c", 6, 0x380, kCHANGE_WILBUR_ANIMATION);
			} else {
				series_stream_with_breaks(SERIES2, "612wi07c", 6, 0x380, kCHANGE_WILBUR_ANIMATION);
			}

			if (_G(flags)[V262] >= 4)
				_G(flags)[V262] = 0;
			else
				++_G(flags)[V262];
			break;

		case 14:
			digi_unload("602_006");
			_G(wilbur_should) = 15;
			_series9 = series_play("602wi7bs", 0x700);
			break;

		case 15:
			digi_unload_stream_breaks(SERIES1);
			digi_unload_stream_breaks(SERIES2);
			{
				static const char *NAMES[10] = {
					"602w017a", "602w017b", "602w017c", "602w017d", "602w017e", "602w017f",
					"602w016a", "602w016b", "602w016c", "602w016d"
				};
				for (int i = 0; i < 10; ++i)
					digi_unload(NAMES[i]);
			}

			series_unload(_sectionSeries1);
			series_unload(_sectionSeries2);
			series_unload(_sectionSeries3);

			if (_G(game).room_id == 602) {
				_mouseWheel = series_show("602wheel", 0x6ff, 0, -1, -1, 0);
			} else {
				_mouseWheel = series_show("612wheel", 0x6ff, 0, -1, -1, 0);
			}

			_G(flags)[V260] = 1;
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("602w020");
			player_set_commands_allowed(true);
			break;

		case 18:
			if (inv_player_has("KIBBLE")) {
				wilbur_speech("602w010");
			} else {
				player_set_commands_allowed(false);
				ws_hide_walker();
				series_play_with_breaks(PLAY15, "602wi10", 0x500, 7, 3);
			}
			break;

		case 20:
			term_message("Slip on kibble!");
			player_set_commands_allowed(false);
			player_update_info();
			_G(wilbur_should) = 21;
			ws_walk(_G(player_info).x + 1, 304, 0, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 21:
			player_set_commands_allowed(false);
			ws_hide_walker();
			Section6::_savedX = _G(player_info).x - 343;

			_G(wilbur_should) = 46;
			_test1 = imath_ranged_rand(0, 5);
			series_play_with_breaks((_G(game).room_id == 602) ? PLAY16 : PLAY17,
				"602wi13", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, Section6::_savedX, 0);
			break;

		case 22:
			ws_demand_location(423, 303, 9);
			player_set_commands_allowed(false);
			ws_hide_walker();

			_G(flags)[V255] = 1;
			_G(kernel).call_daemon_every_loop = true;
			_G(wilbur_should) = 10001;

			series_play_with_breaks(PLAY18, "602wi20", 0x700, 9, 3);
			break;

		case 23:
			_G(flags)[V277] = 6003;

			if (_G(flags)[V278] == 1) {
				ws_demand_location(339, 348);
			} else {
				ws_demand_location(225, 350);
			}

			ws_demand_facing(kCHANGE_DOOR_ANIMATION);
			ws_hide_walker();
			terminateMachineAndNull(_series8);

			if (_G(flags)[V278]) {
				_motorShould = 59;
				series_play_with_breaks(PLAY10, "612wi17", 0x700, 1, 3, 6, 100, 114, -2);
			} else {
				_G(wilbur_should) = 24;
				series_play_with_breaks(PLAY9, "612wi17", 0x700, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 24:
			_motorShould = 57;
			kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);

			if (_G(flags)[V280] == 1) {
				_G(wilbur_should) = 10001;
			} else {
				_G(flags)[V280] = 1;
				_G(wilbur_should) = 34;
			}

			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 25:
			player_set_commands_allowed(false);
			++_G(flags)[V279];

			ws_demand_location(200, 321, 3);
			ws_hide_walker();
			_G(wilbur_should) = 44;

			series_play_with_breaks(PLAY19, "612wi14", 0x600, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 26:
			player_set_commands_allowed(false);
			ws_demand_facing(3);
			ws_hide_walker();

			if (_G(flags)[V278]) {
				_G(wilbur_should) = 10001;
				_test2 = imath_ranged_rand(0, 1);
				series_play_with_breaks(PLAY20, "612wi15", 0x600, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 114, 0);
			} else {
				_G(wilbur_should) = 40;
				_test2 = imath_ranged_rand(0, 1);
				series_play_with_breaks(PLAY20, "612wi15", 0x600, kCHANGE_WILBUR_ANIMATION, 3, 6);
			}
			break;

		case 27:
			player_set_commands_allowed(false);
			ws_demand_location(314, 319, 3);
			ws_hide_walker();
			_G(flags)[kStandingOnKibble] = 0;

			intr_remove_no_walk_rect(_walk1);
			_walk1 = intr_add_no_walk_rect(322, 304, 472, 329, 312, 320);
			terminateMachineAndNull(_series8);

			if (_G(flags)[V277] == 6003) {
				digi_preload("602_004");
				digi_preload("602_005");
				_motorShould = 58;
				_G(wilbur_should) = 29;
				series_play_with_breaks(PLAY22, "612wi18", 0x600, kCHANGE_WILBUR_ANIMATION, 3);
			} else {
				_G(wilbur_should) = 28;
				series_play_with_breaks(PLAY21, "612wi16", 0x700, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 28:
			_G(wilbur_should) = 41;
			_motorShould = 56;
			kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 29:
			_G(wilbur_should) = 41;
			_motorShould = 58;
			kernel_trigger_dispatch_now(kCHANGE_MOTOR_ANIMATION);
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 30:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("602w043");
			break;

		case 31:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("602w044");
			break;

		case 32:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("602w045");
			break;

		case 33:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("602w046");
			break;

		case 34:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("612w065", kCHANGE_WILBUR_ANIMATION);
			break;

		case 35:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("612w065");
			break;

		case 37:
			_G(wilbur_should) = 23;
			player_set_commands_allowed(false);
			wilbur_speech("612w064", kCHANGE_WILBUR_ANIMATION);
			break;

		case 38:
			player_set_commands_allowed(true);
			ws_unhide_walker();

			if (!_G(flags)[V261]) {
				_G(flags)[V261] = 1;
				wilbur_speech("602w031", kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 39:
			_G(wilbur_should) = 27;
			player_set_commands_allowed(false);
			wilbur_speech("612w056", kCHANGE_WILBUR_ANIMATION);
			break;

		case 40:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("612w055");
			break;

		case 41:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("612w057", kCHANGE_WILBUR_ANIMATION);
			break;

		case 42:
			_G(wilbur_should) = 10002;
			player_set_commands_allowed(false);
			ws_unhide_walker();
			wilbur_speech("602w033", 5);
			break;

		case 43:
			wilbur_speech("600w003");
			_G(wilbur_should) = 10001;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 44:
			_G(wilbur_should) = 10001;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

			if (_G(flags)[V279] > 4)
				wilbur_speech("612w066");
			break;

		case 45:
			_G(wilbur_should) = 10001;
			wilbur_speech("602w003");
			break;

		case 46:
			_G(wilbur_should) = 10002;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("600w012");
			break;

		case 47:
			if (!_G(flags)[kGerbilCageDoor]) {
				player_set_commands_allowed(false);
				ws_demand_location(191, 277, 3);
				ws_hide_walker();
				_G(wilbur_should) = 38;

				series_play_with_breaks(_G(flags)[V261] ? PLAY14 : PLAY13,
					"602wi09", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 10001:
			if (_G(flags)[V277] == 6003 && _G(flags)[V278] == 1 && _G(flags)[kGerbilCageDoor] == 0)
				kernel_trigger_dispatch_now(3);

			_G(kernel).continue_handling_trigger = true;
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCALLED_EACH_LOOP:
		if (_G(player).walker_in_this_scene) {
			player_update_info();

			if (_G(player_info).x >= (_kibbleOffset + 330) && _G(player_info).x < 396 &&
					_G(player_info).y > 289 && _G(player_info).y < 305 &&
					_G(flags)[V255] == 1 && _G(flags)[V278] == 0 &&
					_G(player_info).facing > 2 && _G(player_info).facing < 7) {
				if (_G(flags)[kStandingOnKibble]) {
					_G(flags)[kStandingOnKibble] = 1;
				} else {
					_G(flags)[kStandingOnKibble] = 1;
					term_message("Wilbur now slips on kibble!");
					intr_cancel_sentence();
					_G(wilbur_should) = 20;
					kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				}
			} else {
				_G(flags)[kStandingOnKibble] = 0;
			}
		}
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

	} else if (_G(flags)[kHampsterState] == 6006 && (player_said("GEAR", "TUBE") || player_said("CLIMB IN"))) {
		wilbur_speech("600w003");
		intr_cancel_sentence();

	} else if (player_said("KIBBLE") && (player_said("FLOOR ") || player_said("FLOOR  ") || player_said("FLOOR"))) {
		if (_G(flags)[V278] == 0) {
			if (_G(flags)[V255] == 0) {
				_G(wilbur_should) = 22;
				player_hotspot_walk_override(423, 303, 9, kCHANGE_WILBUR_ANIMATION);
			} else {
				wilbur_speech("600w008z");
			}
		}
	} else if (player_said("RAY GUN", "KIBBLE TRAY")) {
		_G(wilbur_should) = 6;
		if (_G(game).room_id == 602) {
			player_hotspot_walk_override(172, 325, 9, kCHANGE_WILBUR_ANIMATION);
		} else {
			player_hotspot_walk_override(151, 315, 9, kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("RAY GUN", "KIBBLE ")) {
		_G(wilbur_should) = 7;
		player_hotspot_walk_override(151, 315, 9, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "EXERCISE WHEEL") && !_G(flags)[V278]) {
		_G(wilbur_should) = 8;
		player_hotspot_walk_override(409, 359, 2, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "DOOR")) {
		_G(wilbur_should) = 9;
		player_hotspot_walk_override(331, 303, 10, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "GERBILS")) {
		_G(wilbur_should) = 10;
		player_hotspot_walk_override(315, 317, 10, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "DOOR")) {
		_G(wilbur_should) = 47;
		player_hotspot_walk_override(257, 290, 10, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "MOTOR") || player_said("GEAR", "MOTOR ")) {
		if (_G(flags)[V278] == 0) {
			if (_G(flags)[V277] == 6001) {
				_G(wilbur_should) = 25;
				player_hotspot_walk_override(200, 321, 3, kCHANGE_WILBUR_ANIMATION);
			}
			if (_G(flags)[V277] == 6002 || _G(flags)[V277] == 6003) {
				if (_G(flags)[V255] == 1) {
					_G(wilbur_should) = 39;
					player_hotspot_walk_override(200, 321, 3, kCHANGE_WILBUR_ANIMATION);
				} else {
					_G(wilbur_should) = 26;
					player_hotspot_walk_override(200, 321, 3, kCHANGE_WILBUR_ANIMATION);
				}
			}
		} else {
			_G(wilbur_should) = 26;
			player_hotspot_walk_override(314, 319, 3, kCHANGE_WILBUR_ANIMATION);
		}
	} else if ((player_said("PANTYHOSE", "MOTOR") || player_said("PANTYHOSE", "MOTOR ")) &&
			_G(flags)[V277] == 6002) {
		_G(wilbur_should) = 37;
		if (_G(flags)[V278]) {
			player_hotspot_walk_override(313, 319, 3, kCHANGE_WILBUR_ANIMATION);
		} else {
			player_hotspot_walk_override(200, 321, 3, kCHANGE_WILBUR_ANIMATION);
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
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
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "PEANUT")) {
		wilbur_speech("602w047");
		intr_cancel_sentence();
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "TUBE") || player_said("CLIMB IN", "TUBE")) {
		_G(flags)[V246] = 1;
		Section6::_state2 = 2;
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("EXIT")) {
		_G(wilbur_should) = 42;
		ws_turn_to_face(5, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("PANTYHOSE", "MOTOR") || player_said("PANTYHOSE", "MOTOR ")) {
		if (_G(flags)[V277] == 6001) {
			_G(wilbur_should) = 25;
			if (_G(flags)[V278]) {
				player_hotspot_walk_override(314, 321, 3, kCHANGE_WILBUR_ANIMATION);
			} else {
				player_hotspot_walk_override(200, 321, 3, kCHANGE_WILBUR_ANIMATION);
			}
		}

		if (_G(flags)[V280]) {
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			wilbur_speech("612w064", kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("GEAR", "EXERCISE WHEEL")) {
		_G(wilbur_should) = 11;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "DOOR")) {
		_G(wilbur_should) = 47;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("KIBBLE ", "TAKE")) {
		_G(wilbur_should) = 18;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

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
