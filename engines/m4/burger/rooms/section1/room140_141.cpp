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

#include "m4/burger/rooms/section1/room140_141.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_TRUFFLES_ANIMATION = 6,
	kCHANGE_ELMO_ANIMATION = 7,
	kCHANGE_WILBUR_ANIMATION = 8
};

static const char *SAID[][4] = {
	{ "TRUFFLES", "141W002", "141W003", "141W003" },
	{ "ISLAND",   "141W004", "141W003", "141W003" },
	{ "CABIN",    "141W005", "141W003", "141W006" },
	{ "GARDEN",   "141W007", "141W003", "141W006" },
	{ "TROUGH",   "141W008", "141W006", "141W006" },
	{ "SIGN",     "141W009", "141W006", "141W006" },
	{ "TREES",    "141W010", "141W003", "141W003" },
	{ "WATER",    "141W011", "141W003", "141W012" },
	{ "ROCKS",    "141W013", "141W003", nullptr   },
	{ "DOCK",     "141W002", nullptr,   nullptr   },
	{ "HANLON'S POINT", nullptr, "141W003", "141W003" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY5[] = {
	{  0,   7, nullptr,   1, 255, -1,    0, 0, nullptr, 0 },
	{  8,  10, "999blow", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 10,  11, nullptr,   1, 255, 14,    0, 0, nullptr, 0 },
	{ 12,  -1, nullptr,   1, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY6[] = {
	{ 0, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, "999blow", 1, 125, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, 15, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 1, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr,   1, 255, 20, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY7[] = {
	{ 5, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 5, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesStreamBreak SERIES1[] = {
	{  9, "141_005", 1, 255, -1, 0, nullptr, 0 },
	{ 18, nullptr,   2, 255,  1, 0, nullptr, 0 },
	{ -1, nullptr,   0,   0, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{ 5, "141_007", 1, 255, -1, 0, 0, 0 },
	{ 9, 0, 2, 255, 3, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{  0, "140_002", 2, 255, -1, 0, nullptr, 0 },
	{ 14, "140_002", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES4[] = {
	{  0, "140_002", 2, 255, -1, 0, nullptr, 0 },
	{ 10, nullptr,   2, 255, 13, 0, &Flags::_flags[kPerkinsLostIsland], 0 },
	{ 25, "140_002", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room140_141::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room140_141::init() {
	digi_preload("144_001");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		player_set_commands_allowed(false);

	if (!_G(flags)[kPerkinsLostIsland]) {
		digi_preload("140t001a", 140);
		digi_preload("140t001b", 140);
		digi_preload("140t001c", 140);
		digi_preload("140t001d", 140);
		digi_preload("140t001e", 140);
		digi_preload("140t001f", 140);

		series_load("140tr01");
		series_load("140tr01s");
		series_load("140tr02");
		series_load("140tr02s");
		series_load("140tr03");
		series_load("140tr03s");
	}

	series_load("140tr04");
	series_load("140tr04s");
	series_load("140tr05");
	series_load("140tr05s");
	series_load("140tr06");
	series_load("140tr06s");
	series_load("140tr07");
	series_load("140tr07s");

	if (!_G(flags)[kPerkinsLostIsland]) {
		_series1 = series_play(_G(flags)[V000] == 1002 ? "141pboat" : "140pboat",
			0xf00, 0, -1, 10, -1, 100, 0, 0, 0, 3);

		series_load("140pe01");
		series_load("140pe01s");
		loadSeries();
		digi_preload("20n01011", 140);
		digi_preload("20n01012", 140);
		digi_preload("20n01013", 140);
		digi_preload("20n01014", 140);

		_elmoMode = 14;
		_elmoShould = 14;
		kernel_trigger_dispatch_now(7);
	}

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		_series2 = series_play("141wave", 0xf00, 0, -1, 10, -1, 100, 0, 0, 0, 3);
		_trufflesMode = 9;
		_trufflesShould = 9;
		_wilburMode = 52;
		_wilburShould = 52;

	} else {
		_wilburMode = 52;
		_wilburShould = 50;

		if (_G(flags)[kPerkinsLostIsland]) {
			_trufflesMode = 1;
			_trufflesShould = 8;
		} else {
			_trufflesMode = 1;
			_trufflesShould = 1;
			_trufflesFrame = 0;
		}
	}

	kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	kernel_trigger_dispatch_now(kCHANGE_TRUFFLES_ANIMATION);
	digi_play_loop("144_001", 3, 180);
}

void Room140_141::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 1:
		_flag2 = true;
		digi_play("141_002", 2, 200, 2);
		break;

	case 2:
		digi_play_loop("141_003", 2, 200, -1);
		break;

	case 3:
		digi_play("141_006", 2, 200, 4);
		break;

	case 4:
		_flag2 = false;
		break;

	case 5:
		_wilburShould = 52;
		player_set_commands_allowed(true);
		break;

	case kCHANGE_TRUFFLES_ANIMATION:
		switch (_trufflesMode) {
		case 1:
			switch (_trufflesShould) {
			case 1:
				_flag1 = true;

				if (imath_ranged_rand(1, 8) == 1) {
					_trufflesMode = 2;
					_trufflesFrame = 0;
					Series::series_play("140tr01", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 1, 1);
				} else {
					Series::series_play("140tr01", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				}
				break;

			case 5:
				Series::series_play("140tr01", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				_elmoShould = 42;
				break;

			case 6:
				Series::series_play("140tr01", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 7:
				digi_play("140t002", 2, 255, 11);
				Series::series_play("140tr02", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 8, 0, 100, 0, 0, 0, 4);
				break;

			case 8:
				_trufflesMode = 9;
				_trufflesShould = 9;
				series_play_with_breaks(PLAY1, "140tr03", 0xa00, 6, 3, 6, 100, 0, 0);
				break;

			default:
				break;
			}
			break;

		case 2:
			if (_trufflesShould == 1) {
				playRandom();

				if (imath_ranged_rand(1, 3) == 1) {
					if (--_trufflesFrame <= 1)
						_trufflesFrame = 3;

				} else {
					if (++_trufflesFrame >= 6) {
						_trufflesFrame = 5;
						_trufflesMode = 4;
					}
					if (_trufflesFrame < 1)
						_trufflesFrame = 1;
				}

				Series::series_play("140tr01", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, _trufflesFrame, _trufflesFrame);

			} else {
				_trufflesMode = 4;
				Series::series_play("140tr01", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 6, 6);
			}
			break;

		case 3:
			if (_trufflesShould == 1) {
				if (imath_ranged_rand(1, 15) == 1) {
					digi_stop(2);
					_trufflesMode = 1;
					Series::series_play("140tr01", 0xa00, 0, 6, 10, 0, 100, 0, 0, 0, 0);
				} else {
					playRandom();
					frame = imath_ranged_rand(7, 8);
					Series::series_play("140tr01", 0xa00, 0, 6, 10, 0, 100, 0, 0, frame, frame);
				}
			} else {
				digi_stop(2);
				_trufflesMode = 1;
				Series::series_play("140tr01", 0xa00, 0, 6, 10, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 4:
			_trufflesMode = 3;
			Series::series_play("140tr01", 0xa00, 0, 6, 6, 0, 100, 0, 0, 9, 11);
			break;

		case 7:
			if (_trufflesShould == 7) {
				frame = imath_ranged_rand(5, 6);
				Series::series_play("140tr02", 0xa00, 0, 6, 7, 0, 100, 0, 0, frame, frame);

			} else {
				Series::series_play("140tr02", 0xa00, 0, 6, 7, 0, 100, 0, 0, 7, 8);
			}
			break;

		case 9:
			switch (_trufflesShould) {
			case 10:
				digi_play("140t002", 2, 255, 12);
				_trufflesMode = 10;
				Series::series_play("140tr06", 0xa00, 0, 6, 8, 0, 100, 0, 0, 0, 4);
				break;

			case 12:
				digi_play(Common::String::format("140t004%c", 'a' + imath_ranged_rand(0, 3)).c_str(),
					2, 255, 10);
				_trufflesMode = 12;
				Series::series_play("140tr07", 0xa00, 0, 6, 7, 0, 100, 0, 0, 0, 3);

				if (player_said("gear", "dock") || player_said("try to dock")) {
					kernel_timing_trigger(90, 16);
				}
				break;

			default:
				if (imath_ranged_rand(1, 10) == 1) {
					_flag1 = true;
					playRandom();
					_trufflesMode = 11;
					Series::series_play("140tr05", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION,10, 0, 100, 0, 0, 0, 2);

				} else {
					Series::series_play("140tr04", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				}
				break;
			}
			break;

		case 10:
			if (_trufflesShould == 10) {
				frame = imath_ranged_rand(5, 6);
				Series::series_play("140tr06", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 7, 0, 100, 0, 0, frame, frame);

			} else {
				Series::series_play("140tr06", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 7, 0, 100, 0, 0, 7, 8);
			}
			break;

		case 11:
			if (_trufflesShould == 11) {
				if (imath_ranged_rand(1, 10) == 1) {
					if (!_flag2)
						digi_stop(2);

					_trufflesMode = 9;
					Series::series_play("140tr05", 0xa00, 2, kCHANGE_TRUFFLES_ANIMATION, 10, 0, 100, 0, 0, 0, 2);
				} else {
					playRandom();
					frame = imath_ranged_rand(3, 5);
					Series::series_play("140tr05", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION,10, 0, 100, 0, 0, frame, frame);
				}
			} else {
				_trufflesMode = 9;
				Series::series_play("140tr05", 0xa00, 2, kCHANGE_TRUFFLES_ANIMATION, 7, 0, 100, 0, 0, 0, 2);
			}
			break;

		case 12:
			if (_trufflesShould == 12) {
				frame = imath_ranged_rand(4, 6);
				Series::series_play("140tr07", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 7, 0, 100, 0, 0, frame, frame);

			} else {
				_trufflesMode = 9;
				Series::series_play("140tr07", 0xa00, 0, kCHANGE_TRUFFLES_ANIMATION, 7, 0, 100, 0, 0, 7, 8);
			}
			break;

		default:
			break;
		}
		break;

	case kCHANGE_ELMO_ANIMATION:
		switch (_elmoMode) {
		case 14:
			switch (_elmoShould) {
			case 14:
				Series::series_play("140pe01", 0xa00, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 1, 1);
				break;

			case 15:
				_elmoShould = 16;
				series_play_with_breaks(PLAY2, "140pe01", 0x500, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 16:
				_elmoShould = 17;
				series_play_with_breaks(PLAY3, "140pe02", 0x500, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 17:
				_elmoShould = _elmoMode = 18;
				kernel_trigger_dispatch_now(kCHANGE_ELMO_ANIMATION);
				conv_resume_curr();
				break;

			default:
				break;
			}
			break;

		case 18:
			switch (_elmoShould) {
			case 18:
				Series::series_play("140pe03", 0x500, 0, kCHANGE_ELMO_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 19:
				Series::series_play("140pe05", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 1, 1);
				break;

			case 20:
				frame = imath_ranged_rand(0, 4);
				Series::series_play("140pe04", 0x500, 0, kCHANGE_ELMO_ANIMATION, 5, 0, 100, 0, 0, frame, frame);
				break;

			case 21:
				_elmoShould = 22;
				Series::series_play("140pe06", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 22:
				_elmoShould = 23;
				Series::series_play("140pe06", 0x500, 0, kCHANGE_ELMO_ANIMATION, 50, 0, 100, 0, 0, 3, 3);
				break;

			case 23:
				_elmoShould = 18;
				Series::series_play("140pe06", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 5, 7);
				conv_resume_curr();
				break;

			case 24:
				_elmoShould = 24;
				Series::series_play("140pe06", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 5);
				kernel_timing_trigger(60, 19);
				break;

			case 25:
				_elmoShould = 26;
				Series::series_play("140pe08", 0x500, 0, kCHANGE_ELMO_ANIMATION, 3, 0, 100, 0, 0, 0, 7);
				break;

			case 26:
				_elmoMode = _elmoShould = 27;
				Series::series_play("140pe08", 0x500, 0, kCHANGE_ELMO_ANIMATION, 10, 0, 100, 0, 0, 10, 10);
				conv_resume_curr();
				break;

			default:
				_elmoMode = 27;
				Series::series_play("140pe08", 0x500, 0, kCHANGE_ELMO_ANIMATION, 3, 0, 100, 0, 0, 0, 7);
				break;
			}
			break;

		case 24:
			if (_elmoShould == 24) {
				if (imath_ranged_rand(1, 8) == 1) {
					digi_play("140_003", 1, 200);
					Series::series_play("140pe07", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 5);
				} else {
					Series::series_play("140pe03", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				}
			}
			break;

		case 27:
			switch (_elmoShould) {
			case 18:
			case 19:
			case 20:
			case 24:
			case 39:
				_elmoMode = 18;
				Series::series_play("140pe15", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				break;

			case 27:
				Series::series_play("140pe08", 0x500, 0, kCHANGE_ELMO_ANIMATION, 10, 0, 100, 0, 0, 10, 10);
				break;

			case 28:
				frame = imath_ranged_rand(8, 10);
				Series::series_play("140pe08", 0x500, 0, kCHANGE_ELMO_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 29:
				_elmoMode = 29;
				Series::series_play("140pe11", 0x500, 0, kCHANGE_ELMO_ANIMATION, 4, 0, 100, 0, 0, 0, 3);
				break;

			case 31:
				Series::series_play("140pe14", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				break;

			case 32:
				series_load("141pe24");
				terminateMachineAndNull(_series1);
				_elmoShould = 33;

				digi_preload("141_002");
				digi_preload("141_003");
				digi_preload("141_006");
				digi_preload_stream_breaks(SERIES1);
				series_stream_with_breaks(SERIES1, "141pe22", 6, 0xf00, 7);
				series_play("141pe22s", 0xf01);
				break;

			case 33:
				_elmoShould = 45;
				_elmoMode = 45;
				series_play("141pe24", 0xf00, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				conv_resume_curr();
				break;

			case 34:
				_elmoShould = 35;
				series_play("140pe13", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			case 35:
				_elmoShould = 36;
				Series::series_play("140pe13", 0x500, 0, kCHANGE_ELMO_ANIMATION, 120, 0, 100, 0, 0, 1, 1);
				conv_resume_curr();
				break;

			case 36:
				_elmoShould = 27;
				Series::series_play("140pe13", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				conv_resume_curr();
				break;

			default:
				_elmoMode = 40;
				Series::series_play("140pe16", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 29:
			if (_elmoShould == 29) {
				frame = imath_ranged_rand(4, 6);
				Series::series_play("140pe11", 0x500, 0, kCHANGE_ELMO_ANIMATION, 4, 0, 100, 0, 0, frame, frame);

			} else {
				_elmoMode = 27;
				Series::series_play("140pe11", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 7, 9);
			}
			break;

		case 40:
			switch(_elmoShould) {
			case 38:
				Series::series_play("140pe17", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				conv_resume_curr();
				break;

			case 40:
				Series::series_play("140pe17", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			case 41:
				frame = imath_ranged_rand(0, 4);
				Series::series_play("140pe18", 0x500, 0, kCHANGE_ELMO_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 42:
				_elmoShould = 40;
				_trufflesShould = 6;
				series_play_with_breaks(PLAY5, "140pe20", 0x500, kCHANGE_ELMO_ANIMATION, 3, 6, 100, 0, 0);
				break;

			case 43:
				_elmoShould = 40;
				Series::series_play("140pe19", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				conv_resume_curr();
				break;

			case 44:
				_elmoMode = _elmoShould = 27;
				Series::series_play("140pe21", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			default:
				_elmoMode = 27;
				Series::series_play("140pe21", 0x500, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 45:
			switch (_elmoShould) {
			case 45:
				series_play("141pe24", 0xf00, 0, kCHANGE_ELMO_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				break;

			case 46:
				frame = imath_ranged_rand(0, 3);
				series_play("141pe24", 0xf00, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, frame, frame);
				break;

			case 47:
				_elmoShould = 45;
				series_play_with_breaks(PLAY4, "140pe25", 0x500, kCHANGE_ELMO_ANIMATION, 3, 6, 100, 0, 0);
				break;

			case 48:
				_elmoShould = 49;
				digi_preload_stream_breaks(SERIES2);
				series_stream_with_breaks(SERIES2, "141pe26", 6, 0x100, 7);
				_G(flags)[kPerkinsLostIsland] = 1;
				break;

			case 49:
				player_set_commands_allowed(true);
				digi_unload("141_002");
				digi_unload("141_003");
				break;
			}
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_wilburMode) {
		case 52:
			switch (_wilburShould) {
			case 50:
				_wilburShould = 51;
				_flag2 = 1;
				digi_preload_stream_breaks(&SERIES4[0]);
				series_stream_with_breaks(&SERIES4[0], _G(flags)[V000] == 1002 ? "141wi01" : "140wi01",
					6, 0xf00, kCHANGE_WILBUR_ANIMATION);
				break;

			case 51:
				_flag2 = 0;
				_series2 = series_play(_G(flags)[V000] == 1002 ? "141wave" : "140wave",
					0xf00, 0, -1, 10, -1, 100, 0, 0, 0, 3);
				_wilburShould = 52;
				kernel_trigger_dispatch_now(8);

				if (_G(flags)[kPerkinsLostIsland])
					_G(walker).wilbur_speech("141w001");
				break;

			case 52:
				series_play(_G(flags)[V000] == 1002 ? "141wi02" : "140wi02",
					0x200, 0, kCHANGE_WILBUR_ANIMATION, 30, 0, 100, 0, 0, 0, 0);
				break;

			case 53:
				frame = imath_ranged_rand(0, 4);
				series_play(_G(flags)[V000] == 1002 ? "141wi02" : "140wi02",
					0x200, 0, kCHANGE_WILBUR_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 54:
				_wilburShould = 52;
				series_play(_G(flags)[V000] == 1002 ? "141wi02" : "140wi02",
					0x200, 0, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 9, 11);
				break;

			case 55:
				terminateMachineAndNull(_series2);
				_flag2 = 1;

				series_stream_with_breaks(SERIES3, _G(flags)[V000] == 1002 ? "141wi03" : "140wi03",
					10, 0xf00, 18);
				break;

			case 56:
				terminateMachineAndNull(_series2);
				_flag2 = 1;

				series_stream_with_breaks(SERIES3, _G(flags)[V000] == 1002 ? "141wi03" : "140wi03",
					6, 0xf00, 18);
				break;

			case 57:
				_wilburShould = 52;
				series_play_with_breaks(PLAY7, "141wi02", 0x200, kCHANGE_WILBUR_ANIMATION, 2, 7, 100, 0, 0);
				break;

			case 58:
				digi_preload("100_023");
				_wilburShould = 59;
				series_play("141wi05", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 12);
				break;

			case 59:
				_wilburShould = 60;
				series_play("141wi05", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 13, 18);
				digi_play("100_023", 2, 255);
				break;

			case 60:
				digi_stop(2);
				_wilburMode = _wilburShould = 58;
				kernel_trigger_dispatch_now(8);
				conv_resume_curr();
				break;

			case 62:
				_wilburShould = 52;
				series_play_with_breaks(PLAY6, "141wi06", 0x200, kCHANGE_WILBUR_ANIMATION, 2, 6, 100, 0, 0);
				break;

			default:
				break;
			}
			break;

		case 58:
			if (_wilburShould == 58) {
				series_play("141wi05", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 10, 0, 100, 0, 0, 18, 18);
			} else {
				_wilburMode = 61;
				series_play("141wi05", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 18);
				digi_play("100_023", 2, 255);
			}
			break;

		case 61:
			_wilburMode = 52;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			digi_stop(2);
			digi_unload("100_023");
			break;

		default:
			break;
		}
		break;

	case 9:
		_flag1 = true;
		break;

	case 10:
		_trufflesShould = 9;
		if (!player_said("gear", "dock") && !player_said("try to dock"))
			_G(walker).wilbur_said(SAID);
		break;

	case 11:
		_trufflesShould = 1;
		_elmoShould = 27;
		conv_resume_curr();
		break;

	case 12:
		_trufflesShould = 9;
		break;

	case 13:
		_elmoShould = 15;
		conv_load_and_prepare("conv20", 22);
		conv_export_value_curr(1, _G(flags)[V000] == 1002 ? 1 : 0);
		conv_export_value_curr(inv_player_has("deed") ? 1 : 0, 2);

		if (_G(flags)[V000] == 1002 && inv_player_has("deed")) {
			conv_export_value_curr(1, 5);
		} else {
			conv_export_value_curr(0, 5);
		}

		conv_play_curr();
		break;

	case 14:
		_trufflesShould = 7;
		break;

	case 15:
		_trufflesShould = 10;
		break;

	case 16:
		_G(walker).wilbur_speech_random("141w014a", "141w01b", "141w014c", 17);
		break;

	case 17:
		wilbur_speech("141w015", 5);
		break;

	case 18:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1012);
		break;

	case 19:
		_wilburShould = 56;
		break;

	case 20:
		player_set_commands_allowed(true);
		break;

	case kWILBUR_SPEECH_STARTED:
		_G(kernel).continue_handling_trigger = 1;
		_wilburShould = 53;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room140_141::parser() {
	bool lookFlag = player_said("look") || player_said("look at");

	if (player_said("conv20")) {
		conv20();

	} else if (player_said("whistle") && player_said_any("GEAR", "WILBUR")) {
		player_set_commands_allowed(false);
		_wilburShould = 62;

	} else if (player_said("gear", "dock") || player_said("try to dock")) {
		_trufflesShould = 12;

	} else if (inv_player_has(_G(player).verb) &&
			player_said_any("cabin", "garden", "trough", "sign")) {
		wilbur_speech("141w006");

	} else if (inv_player_has(_G(player).verb) && player_said_any("trees", "water")) {
		wilbur_speech("141w003");

	} else if (player_said("take") && !inv_player_has(_G(player).noun)) {
		if (player_said("trough") || player_said("sign")) {
			_trufflesShould = 12;

		} else if (!_G(walker).wilbur_said(SAID)) {
			goto check_exit;
		}

	} else if (player_said("gear") && !inv_player_has(_G(player).noun)) {
		if (player_said("cabin") || player_said("garden") ||
				player_said("trough") || player_said("sign")) {
			_trufflesShould = 12;

		} else if (!_G(walker).wilbur_said(SAID)) {
			goto check_exit;
		}

	} else if (!_G(walker).wilbur_said(SAID)) {
		goto check_exit;
	}
	goto done;

check_exit:
	if (player_said("exit") || (player_said("hanlon's point") && lookFlag)) {
		player_set_commands_allowed(false);
		_wilburShould = 55;

	} else {
		return;
	}

done:
	_G(player).command_ready = false;
}

void Room140_141::conv20() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 21) {
		if (who <= 0) {
			if (node == 4 && entry == 0) {
				conv_resume_curr();
			} else if (node == 15 && entry == 1) {
				_elmoShould = 43;
			} else if (node == 16) {
				_trufflesShould = 5;
			} else if (node != 0) {
				if (node == 20 && (entry == 2 || entry == 3)) {
					_elmoShould = 40;
					conv_resume_curr();
				} else if (node == 20 && entry == 1) {
					_elmoShould = 43;
				} else if (node == 20 && entry == 6) {
					_elmoShould = 38;
				} else if (node == 20 && entry == 7) {
					_trufflesShould = 8;
					_elmoShould = 32;
				} else if (node == 20 && entry == 8) {
					_elmoShould = 45;
					conv_resume_curr();
				} else if (node == 20 && entry == 9) {
					_elmoShould = 48;
					conv_resume_curr();
				} else if (node == 6 && entry == 1) {
					_elmoShould = 27;
					conv_resume_curr();
				} else if (node == 4 && (entry == 0 || entry == 4)) {
					_elmoShould = 25;
				} else if (node == 6 && entry == 3) {
					_elmoShould = 31;
					_wilburShould = 55;
					conv_resume_curr();
				} else if (node == 7 ||
						(node == 4 && (entry == 1 || entry == 6 || entry == 7 || entry == 9)) ||
						(node == 5 && entry == 1) ||
						(node == 11 && entry == 5) ||
						(node == 8 && (entry == 0 || entry == 2)) ||
						(node == 10 && entry == 1) ||
						(node == 9)) {
					digi_preload("140_003");
					_elmoShould = 24;
					conv_resume_curr();

				} else if ((node == 4 && (entry == 2 || entry == 5 || entry == 10 || entry == 11)) ||
						(node == 15 && entry == 5) ||
						(node == 18 && entry == 1) ||
						(node == 19 && (entry == 2 || entry == 3))) {
					_wilburShould = 55;
					_elmoShould = (_elmoMode == 27 || _elmoMode == 40) ? 27 : 18;
					conv_resume_curr();

				} else {
					_elmoShould = (_elmoMode == 27 || _elmoMode == 40) ? 27 : 18;
					conv_resume_curr();

				}
			}
		} else if (who == 1) {
			if (node == 20 && entry == 1) {
				_wilburShould = 57;
				conv_resume_curr();
			} else if (node == 20 && entry == 5) {
				_wilburShould = 58;
			} else if ((node == 6 && entry == 1) || (node == 8 && entry == 3)) {
				_elmoShould = 25;
				_wilburShould = 52;
			} else if ((node == 4 && entry == 0) || (node == 20 && entry == 0)) {
				_elmoShould = 21;
				_wilburShould = 52;
			} else {
				_wilburShould = (node == 14) ? 55 : 52;
				conv_resume_curr();
			}
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 20 && entry == 7)
				_wilburShould = 52;

			if (node != 0) {
				if (node == 1 || node == 2) {
					_elmoShould = 20;
					digi_unload("20n01011");
					digi_unload("20n01012");
					digi_unload("20n01013");
					digi_unload("20n01014");
				} else if (node == 6 && entry == 1) {
					_elmoShould = 28;
				} else if (node == 20 && entry == 4) {
					_elmoShould = 28;
				} else if (node == 20 && (entry == 2 || entry == 3 || entry == 7)) {
					_elmoShould = 41;
				} else if (node == 20 && (entry == 8 || entry == 9)) {
					_elmoShould = 46;
				} else if (node == 13 || node == 16) {
					_elmoShould = 41;
				} else if (node == 11 && entry == 5) {
					_elmoShould = 20;
				} else if (node == 6 && (entry == 2 || entry == 3)) {
					_elmoShould = 28;
				} else {
					_elmoShould = (_elmoMode == 27) ? 28 : 20;
				}
			}
		} else if (who == 1) {
			_wilburShould = 53;
		}

		digi_play(conv_sound_to_play(), 1, 255, 21, 140);

	} else {
		conv_resume_curr();
	}
}

void Room140_141::playRandom() {
	if (_flag1 && !_flag2) {
		_flag1 = false;
		digi_play(Common::String::format("140t001%c", 'a' + imath_ranged_rand(0, 5)).c_str(),
			2, 100, 9);
	}
}

void Room140_141::loadSeries() {
	static const char *NAMES[34] = {
		"140pe02", "140pe02s", "140pe03", "140pe03s", "140pe04",
		"140pe04s", "140pe06", "140pe06s", "140pe07", "140pe07s",
		"140pe08", "140pe08s", "140pe09", "140pe09s", "140pe11",
		"140pe11s", "140pe13", "140pe13s", "140pe14", "140pe14s",
		"140pe15", "140pe15s", "140pe16", "140pe16s", "140pe17",
		"140pe17s", "140pe18", "140pe18s", "140pe19", "140pe19s",
		"140pe20", "140pe20s", "140pe21", "140pe21s"
	};
	for (int i = 0; i < 34; ++i)
		series_load(NAMES[i]);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
