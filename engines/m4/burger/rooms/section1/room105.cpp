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

#include "m4/burger/rooms/section1/room105.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

#define COMPARE(X) Common::String(conv_sound_to_play()).equalsIgnoreCase(X)

enum {
	kCHANGE_ELGUS_ANIMATION = 1
};

static const char *SAID[][4] = {
	{ "TOWN RECORDS",     nullptr,   nullptr,   nullptr   },
	{ "MAYOR'S OFFICE",   "105W002", nullptr,   nullptr   },
	{ "COMPLAINTS DEPT.", "105W003", nullptr,   nullptr   },
	{ "CHAIR",            nullptr,   "105W004", "105W005" },
	{ "PETUNIA",          "105W006", "105W007", "105W007" },
	{ "PICTURE",          "105W008", "105W004", "105W009" }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 5, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 6, -1, 0, 1, 255, 28, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 21, 31, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 32, 37, "105_009", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 0, 8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 5, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 6, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY5[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY6[] = {
	{ 10, 17, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 28, 29, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, 29, nullptr, 1, 255, 20, 0, 0, nullptr, 0 },
	{ 30, 32, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY7[] = {
	{ 32, 30, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, 28, nullptr, 1, 255, 45, 0, 0, nullptr, 0 },
	{ 19, 23, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY8[] = {
	{ 18, 23, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 24, 28, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY9[] = {
	{  0,  9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 16, 23, nullptr,   1, 255, 45, 0, 0, nullptr, 0 },
	{ 24, 28, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY10[] = {
	{ 0,  6, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 7, -1, nullptr,   1, 255, 27, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY11[] = {
	{ 0,  12, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY12[] = {
	{ 0, -1, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY13[] = {
	{  0,  3, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  4, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY14[] = {
	{  0,  6, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	{  7, 12, nullptr,   1, 255, 22, 0, 0, nullptr, 0 },
	{ 13, 23, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 24, 45, "105_004", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 46, -1, nullptr,   1, 255, 24, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY15[] = {
	{ 0, 3, nullptr, 1, 255, -1, 0,  0, nullptr, 0 },
	{ 3, 3, nullptr, 1, 255, -1, 0, 12, nullptr, 0 },
	{ 4, 4, nullptr, 1, 255, -1, 0, 10, nullptr, 0 },
	{ 3, 0, nullptr, 1, 255, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY16[] = {
	{  0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  3,  6, nullptr,   1, 255, 23, 0, 0, nullptr, 0 },
	{  7, 11, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, "105_015", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, 26, "105_008", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 27, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY17[] = {
	{  0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  3,  6, nullptr,   1, 255, 23, 0, 0, nullptr, 0 },
	{  7, 11, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, "105_016", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, 26, "105_008", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 27, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY18[] = {
	{  0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  3,  6, nullptr,   1, 255, 23, 0, 0, nullptr, 0 },
	{  7, 11, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, "105_017", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, 26, "105_008", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 27, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY19[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY20[] = {
	{ 27, 30, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY21[] = {
	{ 0, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 2, 2, nullptr, 1, 255,  9, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY22[] = {
	{ 0, 7, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 8, 8, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 8, 8, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY23[] = {
	{ 8, 14, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY24[] = {
	{ 0, 13, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY25[] = {
	{ 14, 17, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY26[] = {
	{ 0, 2, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY27[] = {
	{  3,  9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, 38, "100_023", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY28[] = {
	{  0,  3, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  6,  6, nullptr,   1, 255, 35, 0, 0, nullptr, 0 },
	{  7, 13, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 14, 19, "105_006", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 19,  6, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  5,  5, nullptr,   2, 255, 36, 0, 0, nullptr, 0 },
	{  4,  1, nullptr,   2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY29[] = {
	{ 0, 5, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 6, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 5, 5, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 6, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY30[] = {
	{ 7, 17, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY31[] = {
	{ 0, 6, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, 0, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY32[] = {
	{  8, 14, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr, 0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room105::init() {
	_val1 = 1;
	_series5 = 0;
	digi_preload("105_001");

	Common::fill(&_series1[0], &_series1[12], -1);
	Common::fill(&_series2[0], &_series2[4], -1);
	Common::fill(&_series3[0], &_series3[12], -1);
	Common::fill(&_series4[0], &_series4[5], -1);

	if (_G(flags)[kPerkinsLostIsland]) {
		series_play("105do01", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_elgusShould = 59;
		_elgusMode = 59;
		kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);

	} else {
		hotspot_set_active("Angus", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 104:
		ws_demand_location(310, 353, 11);
		_G(wilbur_should) = 62;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	digi_play_loop("105_001", 3, 180);
}

void Room105::daemon() {
	const int ZERO = 0;
	int frame;

	switch (_G(kernel).trigger) {
	case kCHANGE_ELGUS_ANIMATION:
		switch (_elgusMode) {
		case 5:
			switch (_elgusShould) {
			case 1:
				digi_preload("105_011");
				digi_preload("105_012");
				_elgusShould = 2;
				digi_play("105_007", 2, 255, kCHANGE_ELGUS_ANIMATION);
				digi_change_volume(3, 0);
				break;

			case 2:
				_elgusShould = 3;
				series_play_with_breaks(PLAY14, "105eg01", 0xa00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, ZERO);
				break;

			case 3:
				_elgusShould = 5;
				series_play("105eg02", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 10, 0, 100, 0, ZERO, 0, 0);
				series_play("105eg02s", 0xa01, 0, -1, 10, 0, 100, 0, ZERO, 0, 0);
				_flag3 = false;
				_flag1 = false;
				_flag4 = false;
				_flag2 = false;
				player_set_commands_allowed(true);

				if (_G(flags)[V033] == 3) {
					complaintsDept();
				} else {
					conv_resume();
				}
				break;

			case 5:
				switch (imath_ranged_rand(1, 20)) {
				case 1:
				case 2:
					series_play("105eg04", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 1, 1);
					series_play("105eg04s", 0xa01, 0, -1, 4, 0, 100, 0, ZERO, 1, 1);
					break;

				case 3:
					series_play_with_breaks(PLAY21, "105eg03", 0xa00, 1, 3, 9, 100, 0, ZERO);
					break;

				default:
					series_play("105eg04", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 30, 0, 100, 0, ZERO, 0, 0);
					series_play("105eg04s", 0xa01, 0, -1, 30, 0, 100, 0, ZERO, 0, 0);
					break;
				}
				break;

			case 6:
				_elgusShould = 5;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
				conv_resume();
				break;

			case 7:
				_elgusMode = 7;
				series_play("105eg05", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 0);
				series_play("105eg05s", 0xa01, 0, -1, 4, 0, 100, 0, ZERO, 0, 0);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 8:
				_elgusMode = 8;
				series_play("105eg06", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 6, 0, 100, 0, ZERO, 0, 1);
				series_play("105eg06s", 0xa01, 0, -1, 6, 0, 100, 0, ZERO, 0, 1);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 9:
				_elgusShould = 10;
				series_play_with_breaks(PLAY15, "105eg07", 0xa00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, ZERO);
				break;

			case 10:
				_elgusShould = 7;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 14:
				_elgusShould = 15;
				series_play("105eg08", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 6, 0, 100, 0, ZERO, 0, 5);
				series_play("105eg08s", 0xa01, 0, -1, 6, 0, 100, 0, ZERO, 0, 5);
				break;

			case 15:
				_elgusShould = 16;
				_elgusMode = 16;
				series_play("105eg08", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 6, 0, 100, 0, ZERO, 6, 8);
				series_play("105eg08s", 0xa01, 0, -1, 6, 0, 100, 0, ZERO, 6, 8);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 18: {
				const seriesPlayBreak *PLAYS[3] = { PLAY16, PLAY17, PLAY18 };
				_elgusShould = 21;
				series_play_with_breaks(PLAYS[imath_ranged_rand(0, 2)],
					"105eg10", 0xa00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, ZERO);
				break;
			}

			case 19:
				_elgusShould = 20;
				series_play_with_breaks(PLAY19, "105eg11", 0xa00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, ZERO);
				break;

			case 20:
				_elgusShould = 21;
				series_play_with_breaks(PLAY20, "105eg10", 0xa00, kCHANGE_ELGUS_ANIMATION, 3, 4, 100, 0, ZERO);
				break;

			case 21:
				kernel_timing_trigger(60, 8);
				_elgusShould = 22;
				kernel_timing_trigger(1, 1);
				break;

			case 22:
				unloadAssets3();
				digi_unload("11n1402");

				if (_flag4) {
					_flag3 = false;
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					mayorsDoor();
					mayorsOffice();
					kernel_timing_trigger(60, 12);
				} else {
					player_set_commands_allowed(true);
					_G(flags)[kDisableFootsteps] = 0;
				}
				break;

			default:
				break;
			}
			break;

		case 7:
			if (_elgusShould == 7) {
				_series6 = series_play("105eg05", 0xa00, 4, -1, 4, -1, 100, 0, ZERO, 0, 5);
				_series7 = series_play("105eg05s", 0xa01, 4, -1, 4, -1, 100, 0, ZERO, 0, 5);
			} else {
				terminateMachineAndNull(_series6);
				terminateMachineAndNull(_series7);
				_elgusMode = 5;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			}
			break;

		case 8:
			if (_elgusShould == 8) {
				_series6 = series_play("105eg06", 0xa00, 4, -1, 4, -1, 100, 0, ZERO, 2, 6);
				_series7 = series_play("105eg06s", 0xa01, 4, -1, 4, -1, 100, 0, ZERO, 2, 6);
			} else {
				terminateMachineAndNull(_series6);
				terminateMachineAndNull(_series7);
				_elgusMode = 5;
				series_play("105eg06", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 7, 9);
				series_play("105eg06s", 0xa01, 0, -1, 4, 0, 100, 0, ZERO, 7, 9);
			}
			break;

		case 16:
			if (_elgusShould == 16) {
				_series6 = series_play("105eg06", 0xa00, 4, -1, 4, -1, 100, 0, ZERO, 4, 9);
				_series7 = series_play("105eg06s", 0xa01, 4, -1, 4, -1, 100, 0, ZERO, 4, 9);
			} else {
				terminateMachineAndNull(_series6);
				terminateMachineAndNull(_series7);
				_elgusMode = 5;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			}
			break;

		case 25:
			switch (_elgusShould) {
			case 23:
				_elgusShould = 24;
				series_play_with_breaks(PLAY10, "105mg01", 0xf00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, ZERO);
				break;

			case 24:
				_elgusShould = 25;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);

				if (_G(flags)[V033] == 2) {
					mayorsOffice();
				} else {
					conv_resume();
				}
				break;

			case 25:
				if (imath_ranged_rand(1, 13) == 1) {
					series_play("105mg04", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 0);
					series_play("105mg04s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 0, 0);
				} else {
					series_play("105mg02", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 10, 0, 100, 0, ZERO, 0, 0);
					series_play("105mg02s", 0xf01, 0, -1, 10, 0, 100, 0, ZERO, 0, 0);
				}
				break;

			case 26:
				_elgusShould = 25;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
				conv_resume();
				break;

			case 27:
				_elgusMode = 27;
				series_play("105mg06", 0xf00, 0, 1, kCHANGE_ELGUS_ANIMATION, 0, 100, 0, ZERO, 10, 10);
				series_play("105mg06s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 10, 10);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 28:
				_elgusMode = 27;
				series_play("105mg06", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 9);
				series_play("105mg06s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 0, 9);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 29:
				_elgusShould = 25;
				series_play("105mg03", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 9);
				series_play("105mg03s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 0, 9);
				break;

			case 30:
				_elgusShould = 25;
				series_play("105mg05", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 7);
				series_play("105mg05s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 0, 7);
				break;

			case 31:
				_elgusMode = 31;
				series_play("105mg07", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 2);
				series_play("105mg07s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 0, 2);
				break;

			case 32:
				_elgusShould = 33;
				series_play_with_breaks(PLAY11, "105mg08", 0xf00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, ZERO);
				break;

			case 33:
				_elgusShould = 34;
				kernel_timing_trigger(1, kCHANGE_ELGUS_ANIMATION);
				break;

			case 34:
				unloadAssets1();

				if (_flag1) {
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					loadAssets3();
					complaintsDept();
					kernel_timing_trigger(60, 13);
				} else if (_flag2) {
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					loadAssets4();
					townRecords();
					kernel_timing_trigger(60, 11);
				} else {
					player_set_commands_allowed(true);
					_G(flags)[kDisableFootsteps] = 0;
				}
				break;

			default:
				break;
			}
			break;

		case 27:
			if (_elgusShould == 27) {
				_series6 = series_play("105mg06", 0xf00, 4, -1, 4, -1, 100, 0, ZERO, 10, 14);
				_series7 = series_play("105mg06s", 0xf01, 4, -1, 4, -1, 100, 0, ZERO, 10, 14);
			} else {
				terminateMachineAndNull(_series6);
				terminateMachineAndNull(_series7);
				_elgusMode = 25;

				series_play("105mg02", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 0);
				series_play("105mg02s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 0, 0);
			}
			break;

		case 31:
			if (_elgusShould == 31) {
				_series6 = series_play("105mg07", 0xf00, 4, -1, 4, -1, 100, 0, ZERO, 3, 6);
				_series7 = series_play("105mg07s", 0xf01, 4, -1, 4, -1, 100, 0, ZERO, 3, 6);
			} else {
				terminateMachineAndNull(_series6);
				terminateMachineAndNull(_series7);
				_elgusShould = 32;
				_elgusMode = 25;

				series_play("105mg07", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 7, 8);
				series_play("105mg07s", 0xf01, 0, -1, 4, 0, 100, 0, ZERO, 7, 8);
			}
			break;

		case 35:
			switch (_elgusShould) {
			case 23:
				_elgusShould = 24;
				series_play_with_breaks(PLAY12, "105mg09", 0xf00, kCHANGE_ELGUS_ANIMATION, 2, 6, 100, 0, ZERO);
				break;

			case 24:
				_elgusShould = 35;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);

				if (_G(flags)[V033] == 2) {
					mayorsOffice();
				} else {
					conv_resume();
				}
				break;

			case 32:
				_elgusShould = 33;
				series_play_with_breaks(PLAY13, "105mg14", 0xf00, kCHANGE_ELGUS_ANIMATION, 2, 6, 100, 0, ZERO);
				break;

			case 33:
				_elgusShould = 34;
				kernel_timing_trigger(1, 1);
				break;

			case 34:
				unloadAssets2();

				if (_flag1) {
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					loadAssets3();
					complaintsDept();
					kernel_timing_trigger(60, 13);
				} else if (_flag2) {
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					loadAssets4();
					townRecords();
					kernel_timing_trigger(60, 11);
				} else {
					player_set_commands_allowed(true);
					_G(flags)[kDisableFootsteps] = 0;
				}
				break;

			case 35:
				if (imath_ranged_rand(1, 13) == 1) {
					series_play("105mg11", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 0);
				} else {
					series_play("105mg11", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 10, 0, 100, 0, ZERO, 0, 0);
				}
				break;

			case 36:
				_elgusShould = 35;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
				conv_resume();
				break;

			case 37:
				_elgusMode = 37;
				series_play("105mg12", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 16, 16);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 38:
				_elgusMode = 38;
				series_play("105mg13", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 10);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 39:
				_elgusMode = 37;
				series_play("105mg12", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 0, 15);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			default:
				break;
			}
			break;

		case 37:
			if (_elgusShould == 37) {
				_series6 = series_play("105mg12", 0xf00, 4, -1, 4, -1, 100, 0, ZERO, 16, 18);
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 35;
				series_play("105mg12", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 19, 19);
			}
			break;

		case 38:
			if (_elgusShould == 38) {
				_series6 = series_play("105mg13", 0xf00, 4, -1, 4, -1, 100, 0, ZERO, 11, 14);
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 35;
				series_play("105mg13", 0xf00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, ZERO, 15, 16);
			}
			break;

		case 42:
			switch (_elgusShould) {
			case 40:
				_elgusShould = 41;
				series_play_with_breaks(PLAY1, "105ag01", 0xe00, kCHANGE_ELGUS_ANIMATION, 2, 6, 100, 0, 0);
				break;

			case 41:
				_elgusShould = 42;
				series_play("105ag02", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 10, 0, 100, 0, 0, 0, 0);

				if (_G(flags)[V033] == 1) {
					townRecords();
				} else {
					conv_resume();
				}
				break;

			case 42:
				switch (imath_ranged_rand(1, 35)) {
				case 1:
				case 2:
				case 3:
					series_play_with_breaks(PLAY3, "105ag04", 0xe00, kCHANGE_ELGUS_ANIMATION, 2, 4, 100, 0, 0);
					break;

				case 4:
					series_play_with_breaks(PLAY5, "105ag03", 0xe00, kCHANGE_ELGUS_ANIMATION, 2, 6, 100, 0, 0);
					break;

				default:
					series_play("105ag02", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 15, 0, 100, 0, 0, 0, 0);
					break;
				}
				break;

			case 43:
				_elgusShould = 42;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
				conv_resume();
				break;

			case 44:
				if (imath_ranged_rand(1, 2) == 1) {
					_elgusMode = 45;
					series_play("105ag05", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 4);
				} else {
					_elgusMode = 46;
					series_play("105ag05", 0xe00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 16, 19);
				}

				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 47:
				_elgusMode = 47;
				series_play("105ag06", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 6);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 10);
				break;

			case 49:
				_elgusShould = 50;
				series_play("105ag07", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 9);
				break;

			case 50:
				conv_resume();
				_elgusMode = 49;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
				break;

			case 51:
				_elgusMode = 51;
				series_play("105ag07", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 2);
				break;

			case 52:
				_elgusMode = 52;
				series_play("105ag07", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 24);
				break;

			case 54:
				series_play_with_breaks(PLAY9, "105ag10", 0xe00, 26, 2, 6, 100, 0, 0);
				break;

			case 55:
				series_play_with_breaks(PLAY7, "105ag07", 0xe00, 34, 2, 6, 100, 0, 0);
				break;

			case 56:
				_elgusShould = 57;
				series_play_with_breaks(PLAY8, "105ag09", 0xe00, kCHANGE_ELGUS_ANIMATION, 2, 6, 100, 0, 0);
				break;

			case 57:
				_elgusShould = 58;
				kernel_timing_trigger(1, 1);
				break;

			case 58:
				unloadAssets4();

				if (_flag1) {
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					loadAssets3();
					complaintsDept();
					kernel_timing_trigger(60, 13);

				} else if (_flag4) {
					_flag1 = false;
					_flag4 = false;
					_flag2 = false;
					kernel_timing_trigger(60, 12);

				} else {
					player_set_commands_allowed(true);
					_G(flags)[kDisableFootsteps] = 0;
				}
				break;

			default:
				break;
			}
			break;

		case 45:
			if (_elgusShould == 44) {
				_series6 = series_play("105ag05", 0xe00, 4, -1, 4, -1, 100, 0, 0, 5, 8);
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 42;
				series_play("105ag05", 0xe00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 4);
			}
			break;

		case 46:
			if (_elgusShould == 44) {
				_series6 = series_play("105ag05", 0xe00, 4, -1, 4, -1, 100, 0, 0, 11, 15);
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 42;
				series_play("105ag05", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 16, 19);
			}
			break;

		case 47:
			if (_elgusShould == 47) {
				if (imath_ranged_rand(1, 10) == 1) {
					series_play("105ag06", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 7, 11);
				} else {
					_series6 = series_play("105ag06", 0xe00, 4, -1, 4, -1, 100, 0, 0, 6, 9);
				}
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 42;
				series_play("105ag06", 0xe00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 6);
			}
			break;

		case 48:
			if (_elgusShould == 47) {
				if (imath_ranged_rand(1, 10) == 1) {
					_elgusMode = 47;
					series_play("105ag06", 0xe00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 7, 11);
				} else {
					_series6 = series_play("105ag06", 0xe00, 4, -1, 4, -1, 100, 0, 0, 12, 15);
				}
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 42;
				series_play("105ag06", 0xe00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 16, 18);
			}
			break;

		case 49:
			if (_elgusShould == 50) {
				series_play("105ag07", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 10, 0, 100, 0, 0, 9, 9);
			} else {
				terminateMachineAndNull(_series6);
				_elgusMode = 42;
				series_play_with_breaks(PLAY6, "105ag07", 0xe00, kCHANGE_ELGUS_ANIMATION, 2, 6, 100, 0, 0);
			}
			break;

		case 51:
			switch (_elgusShould) {
			case 51:
				frame = imath_ranged_rand(3, 6);
				series_play("105ag07", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 54:
				_elgusMode = 42;
				_elgusShould = 58;
				series_play_with_breaks(PLAY9, "105ag10", 0xe00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, 0);
				break;

			default:
				_elgusMode = 42;
				_elgusShould = 58;
				series_play_with_breaks(PLAY8, "105ag14", 0xe00, kCHANGE_ELGUS_ANIMATION, 3, 6, 100, 0, 0);
				break;
			}
			break;

		case 53:
			if (_elgusShould == 52) {
				series_play("105ag07", 0xe00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 29, 29);
			} else {
				_elgusMode = 42;
				series_play("105ag07", 0xe00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 30, 33);
			}
			break;

		case 59:
			switch (_elgusShould) {
			case 59:
				frame = imath_ranged_rand(0, 1);
				Series::series_play("105ag12", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 40, 0, 100, 0, 0, frame, frame);
				break;

			case 60:
				_elgusMode = 60;
				Series::series_play("105ag13", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 0);

				if (_digi1) {
					_G(kernel).trigger_mode = _savedMode;
					digi_play(_digi1, 1, 255, _savedTrigger);
					_digi1 = nullptr;
				}
				break;

			case 61:
				_elgusMode = 61;
				series_play("105ag14", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 5);
				series_play("105ag14s", 0xa01, 0, -1, 4, 0, 100, 0, 0, 0, 5);

				if (_digi1) {
					_G(kernel).trigger_mode = _savedMode;
					digi_play(_digi1, 1, 255, _savedTrigger);
					_digi1 = nullptr;
				}
				break;

			default:
				break;
			}
			break;

		case 60:
			if (_elgusShould == 60) {
				frame = imath_ranged_rand(4, 8);
				series_play("105ag13", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				series_play("105ag13s", 0xa01, 0, -1, 4, 0, 100, 0, 0, frame, frame);
			} else {
				_elgusMode = 59;
				series_play("105ag13", 0xa00, 2, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 0, 3);
				series_play("105ag13s", 0xa01, 2, -1, 4, 0, 100, 0, 0, 0, 3);
			}
			break;

		case 61:
			if (_elgusShould == 61) {
				frame = imath_ranged_rand(6, 9);
				series_play("105ag14", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				series_play("105ag14s", 0xa01, 0, -1, 4, 0, 100, 0, 0, frame, frame);
			} else {
				_elgusMode = 59;
				series_play("105ag14", 0xa00, 0, kCHANGE_ELGUS_ANIMATION, 4, 0, 100, 0, 0, 12, 15);
				series_play("105ag14s", 0xa01, 0, -1, 4, 0, 100, 0, 0, 12, 15);
			}
			break;

		default:
			break;
		}
		break;

	case 2:
		if (!_val1) {
			player_set_commands_allowed(true);
		} else {
			_elgusShould = _series5 ? 54 : 56;
			player_set_commands_allowed(false);
		}
		break;

	case 3:
		_elgusShould = 32;
		player_set_commands_allowed(false);
		break;

	case 4:
		digi_unload("105_011");
		digi_unload("105_012");
		_elgusShould = _flag3 ? 19 : 18;
		player_set_commands_allowed(_flag5);
		_flag5 = false;
		break;

	case 5:
		player_set_commands_allowed(true);
		break;

	case 6:
		player_set_commands_allowed(true);
		break;

	case 7:
		digi_play("105_014", 2, 255, 1004);
		break;

	case 8:
		digi_play_loop("105_001", 3, 180);
		break;

	case 9:
		digi_play(imath_ranged_rand(1, 2) == 1 ? "105_011" : "105_012", 2, 180, -1);
		break;

	case 11:
		ws_walk(186, 263, 0, -1, 10);
		break;

	case 12:
		ws_walk(341, 216, nullptr, -1, 11);
		break;

	case 13:
		ws_walk(415, 254, 0, -1, 2);
		break;

	case 15:
		ws_unhide_walker();

		if (_G(flags)[kPerkinsLostIsland]) {
			_savedMode = KT_DAEMON;
			_savedTrigger = 21;
			_digi1 = "105g003";
			_elgusShould = 60;
		} else if (_G(flags)[V033] == 3 && !_G(flags)[V032]) {
			_elgusShould = 1;
			_elgusMode = 5;
			kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
		} else {
			_flag5 = _G(flags)[V032] != 0;
			_flag3 = false;
			_flag1 = false;
			_flag4 = false;
			_flag2 = false;
			complaintsDept();
		}
		break;

	case 16:
		ws_unhide_walker();
		kernel_timing_trigger(1, 17);
		break;

	case 17:
		if (_G(flags)[kPerkinsLostIsland]) {
			_savedMode = KT_DAEMON;
			_savedTrigger = 21;
			_digi1 = "105g002";
			_elgusShould = 60;
		} else if (_G(flags)[V033] != 2) {
			_flag3 = false;
			_flag1 = false;
			_flag4 = false;
			_flag2 = false;
			mayorsOffice();
		} else {
			kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
		}
		break;

	case 18:
		ws_unhide_walker();

		if (_G(flags)[V033] == 1) {
			_elgusShould = 40;
			_elgusMode = 42;
			kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
		} else {
			townRecords();
		}
		break;

	case 19:
		startConv13();
		break;

	case 20:
		_series5 = series_play("105deed", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		break;

	case 21:
		player_set_commands_allowed(true);
		_elgusShould = 59;
		break;

	case 22:
		ws_demand_location(376, 226, 5);
		ws_hide_walker();
		series_play_with_breaks(PLAY30, "105wi06", 0xf00, 25, 3);
		break;

	case 23:
		ws_demand_location(346, 226, 5);
		ws_hide_walker();
		series_play_with_breaks(PLAY30, "105wi06", 0xf00, 25, 3, 6, 100, -30, 0);
		break;

	case 24:
		ws_walk(385, 254, 0, -1, 2);
		break;

	case 25:
		ws_unhide_walker();
		break;

	case 26:
		player_set_commands_allowed(true);
		break;

	case 27:
		ws_demand_location(358, 246, 11);
		ws_hide_walker();
		series_play_with_breaks(PLAY32, "105wi05", 0x100, 25, 3);
		break;

	case 28:
		ws_demand_location(228, 262, 10);
		ws_hide_walker();
		series_play_with_breaks(PLAY23, "105wi01", 0x100, 25, 3);
		break;

	case 29:
		ws_hide_walker();
		_series12 = series_load("105wi07");
		digi_preload("100_023");
		series_play("105wi07", 0x100, 0, 30, 6, 0, 100, -30, 0, 0, 15);
		series_play("105wi07s", 0x101, 0, -1, 6, 0, 100, -30, 0, 0, 15);
		break;

	case 30:
		series_play("105wi07", 0x100, 0, 31, 6, 0, 100, -30, 0, 16, 21);
		series_play("105wi07s", 0x101, 0, -1, 6, 0, 100, -30, 0, 16, 21);
		digi_play("100_023", 2, 255);
		conv_resume_curr();
		break;

	case 31:
		digi_stop(2);
		_series9 = series_play("105wi07", 0x100, 0, 30, 600, -1, 100, -30, 0, 21, 21);
		_series10 = series_play("105wi07s", 0x100, 0, -1, 600, -1, 100, -30, 0, 21, 21);
		break;

	case 32:
		terminateMachineAndNull(_series9);
		terminateMachineAndNull(_series10);
		digi_play("100_023", 2, 255);
		series_play("105wi07", 0x100, 0, 33, 6, 0, 100, -30, 0, 21, 43);
		series_play("105wi07s", 0x101, 0, -1, 6, 0, 100, -30, 0, 21, 43);
		break;

	case 33:
		digi_stop(2);
		digi_unload("100_023");
		series_unload(_series12);
		ws_unhide_walker();
		break;

	case 34:
		_series8 = series_play("105ag07", 0xe00, 0, -1, 600, -1, 100, 0, 0, 23, 23);
		ws_hide_walker();
		series_play_with_breaks(PLAY28, "105wi03", 0x200, 46, 3, 6, 100, 40, 0);
		break;

	case 35:
		terminateMachineAndNull(_series8);
		_series8 = series_play("105ag07", 0xe00, 0, -1, 600, -1, 100, 0, 0, 24, 24);
		break;

	case 36:
		terminateMachineAndNull(_series8);
		_series8 = series_play("105ag07", 0xe00, 0, 37, 6, 0, 100, 0, 0, 25, 29);
		break;

	case 37:
		_series5 = series_play("105deed", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		series_play("105ag07", 0xe00, 0, 38, 6, 0, 100, 0, 0, 31, 32);
		break;

	case 38:
		series_play_with_breaks(PLAY4, "105ag08", 0xe00, 39, 2);
		break;

	case 39:
		_series8 = series_play("105ag08", 0xe00, 0, -1, 600, -1, 100, 0, 0, 6, 6);
		ws_hide_walker();
		series_play_with_breaks(PLAY24, "105wi02", 0x200, 40, 3, 6, 100, 40, 0);
		break;

	case 40:
		terminateMachineAndNull(_series8);
		series_play("105ag08", 0xe00, 0, 41, 7, 0, 100, 0, 0, 11, 15);
		series_play_with_breaks(PLAY25, "105wi02", 0x200, 46, 3, 6, 100, 40, 0);
		break;

	case 41:
		terminateMachineAndNull(_series5);
		_G(flags)[V001] = 0;
		series_play_with_breaks(PLAY2, "105ag08", 0xe00, 42, 2);
		break;

	case 42:
		_series11 = series_load("105ag08");
		_series8 = series_play("105ag08", 0xe00, 0, -1, 600, -1, 100, 0, 0, 37, 37);
		ws_hide_walker();
		series_play_with_breaks(PLAY26, "105wi04", 0x200, 43, 3, 6, 100, 40, 0);
		break;

	case 43:
		terminateMachineAndNull(_series8);
		series_play("105ag08", 0xe00, 0, 44, 6, 0, 100, 0, 0, 38, 42);
		series_play_with_breaks(PLAY27, "105wi04", 0x200, 46, 3, 6, 100, 40, 0);
		inv_give_to_player("deed");			// Finally get the deed after all the above code
		inv_move_object("money", NOWHERE);	// No! My money!
		break;

	case 44:
		series_unload(_series11);
		_elgusShould = 42;
		_elgusMode = 42;
		kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
		conv_resume();
		break;

	case 45:
		terminateMachineAndNull(_series5);
		break;

	case 46:
		ws_unhide_walker();
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 62:
			if (_G(flags)[kPerkinsLostIsland] && !_G(flags)[V034]) {
				player_set_commands_allowed(false);
				_G(flags)[V034] = 1;
				ws_walk(199, 279, 0, 19, 9);

			} else {
				ws_walk(301, 313, 0, -1, 11);
			}
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

void Room105::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("gear", "town records") && !_G(flags)[kPerkinsLostIsland])
		player_hotspot_walk_override(186, 263, 10);

	if (player_said("gear", "mayor's office"))
		player_hotspot_walk_override(341, 216, 11);

	if (player_said("GEAR", "complaints dept."))
		player_hotspot_walk_override(415, 254, 2);
}

void Room105::parser() {
	bool lookFlag = player_said("look") || player_said("look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv12")) {
		conv12();
	} else if (player_said("conv10")) {
		conv10();
	} else if (player_said("conv11")) {
		conv11();
	} else if (player_said("conv13")) {
		conv13();
	} else if (player_said("talk to")) {
		startConv13();
	} else if (lookFlag && player_said("town records")) {
		if (_G(flags)[kPerkinsLostIsland]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				wilbur_speech("105w010");
				break;

			case 1:
				_savedMode = KT_DAEMON;
				_savedTrigger = 21;
				_digi1 = "105g001";
				_elgusShould = 60;
				break;

			default:
				break;
			}
		} else {
			wilbur_speech("105w001");
		}
	} else if (!_G(walker).wilbur_said(SAID)) {
		if (player_said("GEAR", "town records")) {
			if (_G(flags)[kPerkinsLostIsland]) {
				wilbur_speech("105w011");
			} else {
				player_set_commands_allowed(false);
				_G(flags)[kDisableFootsteps] = 1;
				loadAssets4();
				ws_hide_walker();
				series_play_with_breaks(PLAY22, "105wi01", 0xa00, 18, 3);
			}
		} else if (player_said("GEAR", "mayor's office")) {
			player_set_commands_allowed(false);
			ws_hide_walker();
			series_play_with_breaks(PLAY31, "105wi05", 0x100, 16, 3);

			if (!_G(flags)[kPerkinsLostIsland]) {
				_G(flags)[kDisableFootsteps] = 1;
				mayorsDoor();
			}
		} else if (player_said("GEAR", "complaints dept.")) {
			player_set_commands_allowed(false);
			_G(flags)[kDisableFootsteps] = 1;
			loadAssets3();
			digi_preload("11n1402");
			ws_hide_walker();
			series_play_with_breaks(PLAY29, "105wi06", 0xa00, 15, 3);
		} else if (player_said("gear", "town records")) {
			wilbur_speech("105w011");
		} else if (player_said("exit", "STAIRS")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 7);
		} else if (!player_said("take", "baitbox")) {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room105::conv10() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();

	if (_G(kernel).trigger == 10) {
		if (who <= 0) {
			if (node == 1) {
				digi_stop(2);

				if (COMPARE("10n02011")) {
					digi_unload("105gucr1");
				} else if (COMPARE("10n02012")) {
					digi_unload("105gucr2");
				} else {
					digi_unload("105gucr3");
				}

				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			} else {
				_elgusShould = _val8 ? 36 : 26;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			}
		} else {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			conv_resume();
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 23)
				_flag1 = true;
			if (node == 21)
				_flag2 = true;

			if (node == 1) {
				if (COMPARE("10n02011")) {
					digi_preload("105gucr1");
					digi_play("105gucr1", 2, 150);
				} else if (COMPARE("10n02012")) {
					digi_preload("105gucr2");
					digi_play("105gucr2", 2, 150);
				} else if (COMPARE("10n02013")) {
					digi_preload("105gucr3");
					digi_play("105gucr3", 2, 150);
				}

				digi_play(conv_sound_to_play(), 1, 255, 10);

			} else {
				_elgusShould = _val8 ? 37 : 27;
			}
		} else if (who == 1) {
			sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
			digi_play(conv_sound_to_play(), 1, 255, 10);
		}
	} else {
		conv_resume();
	}
}

void Room105::conv11() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 10) {
		if (who <= 0) {
			if (node == 1) {
				digi_stop(2);

				if (COMPARE("10n02011")) {
					digi_unload("105e1cr1");
				} else if (COMPARE("10n02012")) {
					digi_unload("105e1cr2");
				} else if (COMPARE("10n02013")) {
					digi_unload("105e1cr3");
				}

				_elgusShould = 1;
				_elgusMode = 5;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(30, 1);

			} else if ((node == 8 && entry == 1) ||
					(node == 1 && entry == 4 && inv_player_has("deed"))) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 29);
				_elgusShould = 5;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			} else {
				_elgusShould = 6;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			}
		} else if (who == 1) {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, 0, 1);
			conv_resume();
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 9 && entry == 2)
				_flag3 = true;
			if (node == 10)
				_flag4 = true;

			if (node != 7 &&
					(node != 8 || entry != 0) && (node != 9 || entry != 2) &&
					(node != 11 || entry != 3) && (node != 12 || entry != 3)) {
				if (node == 13 && entry == 0) {
					_elgusShould = 9;
				} else if (node == 13 && entry == 1) {
					_elgusShould = 14;
				} else if (node != 1) {
					_elgusShould = 7;
				} else {
					if (COMPARE("10n02011")) {
						digi_preload("105e1cr1");
						digi_play("105e1cr1", 2, 150);
					} else if (COMPARE("10n02012")) {
						digi_preload("105e1cr2");
						digi_play("105e1cr2", 2, 150);
					} else if (COMPARE("10n02013")) {
						digi_preload("105e1cr3");
						digi_play("105e1cr3", 2, 150);
					}

					digi_play(conv_sound_to_play(), 1, 255, 10);
				}
			} else {
				_elgusShould = 8;
			}
		} else if (who == 1) {
			if (node == 13 && entry == 1) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 32);
				_G(kernel).trigger_mode = KT_PARSE;
			} else if (node != 13 || entry != 0) {
				sendWSMessage(0x140000, 0, _G(my_walker), 0, 0, 1);
			}

			digi_play(conv_sound_to_play(), 1, 255, 10);
		}
	} else {
		conv_resume();
	}
}

void Room105::conv12() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 10) {
		if (who <= 0) {
			if (node == 1) {
				digi_stop(2);

 				if (COMPARE("10n02011")) {
					digi_unload("105ancr1");
				} else if (COMPARE("10n02012")) {
					digi_unload("105ancr2");
				} else if (COMPARE("10n02013")) {
					digi_unload("105ancr3");
				}

				_elgusShould = 40;
				_elgusMode = 42;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);

			} else if (node == 7 && entry == 6) {
				_elgusShould = 49;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);

			} else if (node == 15 && entry == 4) {
				_elgusShould = 55;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);

			} else if (node == 9) {
				conv_resume();

			} else {
				_elgusShould = 43;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_ELGUS_ANIMATION);
			}
		} else if (who == 1) {
			if (node == 16 && entry == 1)
				digi_unload("12p1702");

			sendWSMessage(0x150000, 0, _G(my_walker), 0, 0, 1);
			conv_resume();
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 17)
				_flag1 = true;
			if (node == 7 && entry == 7)
				_flag4 = true;

			if (node == 1) {
				if (COMPARE("10n02011")) {
					digi_preload("105ancr1");
					digi_play("105ancr1", 2, 150);
				} else if (COMPARE("10n02012")) {
					digi_preload("105ancr2");
					digi_play("105ancr2", 2, 150);
				} else if (COMPARE("10n02013")) {
					digi_preload("105ancr3");
					digi_play("105ancr3", 2, 150);
				}

				digi_play(conv_sound_to_play(), 1, 255, 10);

			} else if (node == 9) {
				digi_play(conv_sound_to_play(), 1, 255, 10);

			} else {
				_elgusShould = 44;
			}
		} else if (who == 1) {
			if (node == 16 && entry == 1) {
				digi_preload("12p1702");
				_elgusShould = 56;
				_val1 = 0;
			}

			sendWSMessage(0x140000, 0, _G(my_walker), 0, 0, 1);
			digi_play(conv_sound_to_play(), 1, 255, 10);
		}
	} else {
		conv_resume();
	}
}

void Room105::conv13() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();

	if (_G(kernel).trigger == 10) {
		if (who <= 0) {
			_elgusShould = 59;
		} else if (who == 1) {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, 0, 1);
		}

		conv_resume();
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			_elgusShould = (node == 2) ? 61 : 60;
			_savedMode = KT_PARSE;
			_savedTrigger = 10;
			_digi1 = conv_sound_to_play();
		} else if (who == 1) {
			sendWSMessage(0x140000, 0, _G(my_walker), 0, 0, 1);
			digi_play(conv_sound_to_play(), 1, 255, 10);
		}
	} else {
		conv_resume();
	}
}

void Room105::startConv13() {
	_val1 = 1;
	conv_load_and_prepare("conv13", 5);
	conv_play_curr();
}

void Room105::mayorsDoor() {
	if (_G(flags)[V031]) {
		_val8 = 1;
		loadAssets2();
		_elgusShould = 23;
		_elgusMode = 35;
	} else {
		_G(flags)[V031] = 1;
		_val8 = 0;
		loadAssets1();
		_elgusShould = 23;
		_elgusMode = 25;
	}
}

void Room105::loadAssets1() {
	static const char *NAMES[12] = {
		"105mg02", "105mg02s", "105mg03", "105mg03s", "105mg04", "105mg04s",
		"105mg05", "105mg05s", "105mg06", "105mg06s", "105mg07", "105mg07s"
	};

	for (int i = 0; i < 12; ++i)
		_series1[i] = series_load(NAMES[i]);
}

void Room105::loadAssets2() {
	static const char *NAMES[4] = {
		"105mg10", "105mg11", "105mg12", "105mg13"
	};

	for (int i = 0; i < 4; ++i)
		_series2[i] = series_load(NAMES[i]);
}

void Room105::loadAssets3() {
	static const char *NAMES[12] = {
		"105eg02", "105eg02s", "105eg04", "105eg04s", "105eg05", "105eg05s",
		"105eg06", "105eg06s", "105eg07", "105eg07s", "105eg09", "105eg09s"
	};

	for (int i = 0; i < 12; ++i)
		_series3[i] = series_load(NAMES[i]);

	series_load("105eg10");
	series_load("105eg10s");
}

void Room105::loadAssets4() {
	static const char *NAMES[5] = {
		"105ag02", "105ag03", "105ag05", "105ag06", "105ag07"
	};

	for (int i = 0; i < 5; ++i)
		_series1[i] = series_load(NAMES[i]);
}


void Room105::unloadAssets1() {
	for (int i = 0; i < 12; ++i)
		series_unload(_series1[i]);
}

void Room105::unloadAssets2() {
	for (int i = 0; i < 4; ++i)
		series_unload(_series2[i]);
}

void Room105::unloadAssets3() {
	for (int i = 0; i < 12; ++i)
		series_unload(_series3[i]);
}

void Room105::unloadAssets4() {
	for (int i = 0; i < 5; ++i)
		series_unload(_series4[i]);
}

void Room105::townRecords() {
	_val1 = 1;
	conv_load_and_prepare("conv12", 2);
	conv_export_pointer_curr(&_G(flags)[V033], 0);
	conv_export_value_curr(inv_player_has("deed") ? 1 : 0, 2);
	conv_export_value_curr(_G(flags)[V001], 3);
	conv_play_curr();
}

void Room105::complaintsDept() {
	_val1 = 1;
	conv_load_and_prepare("conv11", 4);
	conv_export_pointer_curr(&_G(flags)[V032], 0);
	conv_export_pointer_curr(&_G(flags)[V033], 3);
	conv_export_value_curr(inv_player_has("deed") ? 1 : 0, 5);
	conv_play_curr();
}

void Room105::mayorsOffice() {
	_val1 = 1;
	conv_load_and_prepare("conv10", 3);
	conv_export_value_curr(inv_player_has("deed") ? 1 : 0, 0);
	conv_export_pointer_curr(&_G(flags)[V033], 1);
	conv_export_value_curr(_G(flags)[V016], 4);
	conv_export_value_curr(player_been_here(137) ? 1 : 0, 5);
	conv_play_curr();
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
