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

#include "m4/burger/rooms/section1/room102.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_HARRY_ANIMATION = 3,
	kCHANGE_WILBUR_CHAIR_ANIMATION = 4
};

static const seriesStreamBreak STREAMS1[] = {
	{ 40, nullptr,   1, 255, 12, 0, nullptr, 0 },
	{ 57, nullptr,   1, 255,  1, 0, nullptr, 0 },
	{ 57, "102_034", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS2[] = {
	{ 13, "102_036", 2, 255, -1, 0, nullptr, 0 },
	{ 16, "102_037", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS3[] = {
	{ 13, "102_036", 2, 255, -1, 0, nullptr, 0 },
	{ 16, "102_037", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS4[] = {
	{ 6,"102_035", 2, 255, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS5[] = {
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS6[] = {
	{ 13, "102w010", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS7[] = {
	{ 13, "102w010", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8, 8, nullptr, 1, 255,  2, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0, 18, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  6, 20, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 21, -1, nullptr, 1, 255,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 0,  5, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 14, "102_039", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room102::init() {
	_val1 = 0;
	_series3 = 0;
	_val3 = -1;
	_val4 = 0;
	_val5 = 0;
	_val6 = 0;
	_val8 = 0;
	_val9 = 0;
	_val10 = 99999;
	_G(flags)[kDisableFootsteps] = 1;

	digi_preload("102_001");
	digi_preload("102_002");

	switch (_G(flags)[V012]) {
	case 1:
		_index1 = series_load("102ha01", -1);
		_index2 = series_load("102ha03", -1);
		_harryMode = 35;
		_harryShould = 35;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);

		_wilburChairShould = 58;
		_wilburChairMode = 62;
		kernel_trigger_dispatch_now(4);

		hotspot_set_active("harry", false);
		setup(5, 0);
		break;

	case 2:
		hotspot_set_active("harry", false);
		hotspot_set_active("harry ", false);

		_series1 = series_play("102seat", 0xa00, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_series1s = series_play("102seats", 0xa01, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		setup(5, 0);
		break;

	default:
		_index1 = series_load("102ha01", -1);
		setup(0, 0);

		_harryMode = 19;
		_harryShould = 19;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		++_G(flags)[V015];

		_series1s = series_play("102seats", 0xa01, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		hotspot_set_active("harry ", false);
		break;
	}

	_trigger = -1;

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (_G(flags)[V012] == 1) {
			ws_demand_location(321, 343);
			ws_demand_facing(4);
			ws_hide_walker();
		}
	} else {
		_G(wilbur_should) = 1;
		_val16 = 1;
		digi_preload("102_038");
		digi_play("102_038", 2, 255, kCHANGE_WILBUR_ANIMATION);
		player_set_commands_allowed(false);
	}

	kernel_trigger_dispatch_now(6);
	setupWax();
	setupLaz();
}

void Room102::daemon() {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	int frame;

	switch (_G(kernel).trigger) {
	case 1:
		conv_resume_curr();
		break;

	case 2:
		ws_unhide_walker();
		break;

	case kCHANGE_HARRY_ANIMATION:
		switch (_harryShould) {
		case 19:
			term_message(" mumble or change channel");

			if (timer_read_60() > _val10 &&_harryMode == 19 && player_commands_allowed() &&
					!digi_play_state(1) && INTERFACE_VISIBLE) {
				if (_val8) {
					_harryMode = 26;
					term_message("change channel");
				} else {
					digi_play(getDigi1(_G(flags)[V008]), 2, 255, 18);
					_val9 = 1;
					_harryMode = 20;
					term_message("mumble");
				}
			}

			term_message("!");

			switch (_harryMode) {
			case 19:
				switch (imath_ranged_rand(1, 45)) {
				case 1:
				case 2:
				case 3:
					series_play("102ha01", 0xa00, 1, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, 0, 2);
					break;
				case 6:
					_harryShould = 30;
					series_play("102ha01", 0xa00, 2, kCHANGE_HARRY_ANIMATION, 10, 0, 100, 0, 0, 16, 19);
					break;
				case 7:
					_harryShould = 31;
					series_play("102ha01", 0xa00, 2, kCHANGE_HARRY_ANIMATION, 10, 0, 100, 0, 0, 20, 23);
					break;
				case 8:
					_harryShould = 24;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 1, 1);
					break;
				case 9:
					_harryShould = 25;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 7, 0, 100, 0, 0, 3, 4);
					break;
				case 10:
					_harryShould = 23;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 8, 0, 100, 0, 0, 46, 48);
					break;
				default:
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 40, 0, 100, 0, 0, 0, 0);
					break;
				}
				break;

			case 20:
				_harryShould = 20;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, 24, 24);
				break;

			case 21:
				_harryShould = 21;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, 30, 30);
				break;

			case 26:
				_harryMode = 27;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 8, 14);
				break;

			case 27:
				if (player_commands_allowed() && !digi_play_state(1)) {
					setup();
					_harryMode = 19;
					_harryShould = 26;
					kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				} else {
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 60, 0, 100, 0, 0, 14, 14);
				}
				break;

			case 28:
				_harryMode = 29;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 8, 14);
				break;

			case 29:
				setup(5);
				_harryMode = 32;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 15, 15);
				break;

			case 32:
				series_unload(_index1);
				terminateMachineAndNull(_series1s);
				_harryMode = 33;
				series_play("102ha02s", 0xf01, 0, 11, 6, 0, 100, 0, 0, 0, 41);
				digi_preload_stream_breaks(STREAMS1);
				_stream1 = series_stream_with_breaks(STREAMS1, "102ha02", 6, 0xf00, 3);
				break;

			case 33:
				digi_unload_stream_breaks(STREAMS1);
				_wilburChairMode = 62;
				_wilburChairShould = 58;
				kernel_trigger_dispatch_now(4);
				digi_stop(2);
				_index2 = series_load("102ha03", -1);
				_harryMode = 34;
				Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 34:
				_harryMode = 35;
				_harryShould = 35;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				hotspot_set_active("harry", false);
				hotspot_set_active("harry ", true);
				conv_resume_curr();
				break;

			default:
				break;
			}
			break;

		case 20:
			if (_harryMode == 20) {
				if (_play1) {
					digi_play(_play1, 1, 255, 26);
					_play1 = nullptr;
				}
				if (_series3)
					terminateMachineAndNull(_series3);

				_series3 = series_play("102ha01", 0xa00, 4, -1, 5, -1, 100, 0, 0, 24, 29);
			} else {
				terminateMachineAndNull(_series3);
				_series3 = nullptr;
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, 24, 24);
			}
			break;

		case 21:
			if (_harryMode == 21) {
				if (_play1) {
					digi_play(_play1, 1, 255, 26);
					_play1 = nullptr;
				}

				_series3 = series_play("102ha01", 0xa00, 4, -1, 5, -1, 100, 0, 0, 31, 42);
			} else {
				terminateMachineAndNull(_series3);
				_series3 = nullptr;
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, 30, 30);
			}
			break;

		case 22:
			_harryShould = 23;
			series_play("102ha01", 0xa00, 2, 3, 6, 0, 100, 0, 0, 44, 48);
			break;

		case 23:
			_harryShould = 19;
			series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 44, 46);
			break;

		case 24:
			if (_harryMode == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_harryShould = 19;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				} else {
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 1, 1);
				}
			} else {
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 25:
			if (_harryMode == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_harryShould = 19;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 5, 7);
				} else {
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 4, 4);
				}
			} else {
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 5, 7);
			}
			break;

		case 26:
			if (_harryMode == 19) {
				if (imath_ranged_rand(1, 17) == 1) {
					_harryShould = 19;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 15, 15);
				} else {
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 14, 14);
				}
			} else {
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 15, 15);
			}
			break;

		case 30:
			if (_harryMode == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_harryShould = 19;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 16, 19);
				} else {
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 16, 16);
				}
			} else {
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 16, 19);
			}
			break;

		case 31:
			if (_harryMode == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_harryShould = 19;
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 20, 23);
				} else {
					series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 20, 20);
				}
			} else {
				_harryShould = 19;
				series_play("102ha01", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 20, 23);
			}
			break;

		case 35:
			switch (_harryMode) {
			case 35:
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					if (!digi_play_state(2))
						digi_play("102_030", 2, 255, -1);

					Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 4, 6);
					break;

				case 2:
					if (!digi_play_state(2))
						digi_play("102_031", 2, 255, -1);

					Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 7, 8);
					break;

				case 3:
					if (!digi_play_state(2))
						digi_play("102_032", 2, 255, -1);

					Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 9, 11);
					break;

				case 4:
					if (!digi_play_state(2))
						digi_play("102_033", 2, 255, -1);

					Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 12, 15);
					break;

				default:
					break;
				}

				if (++_val5 > 24 && _val5 != -666 && player_commands_allowed()) {
					_val5 = -666;
					conv_load_and_prepare("conv05", 20, 0);
					conv_export_pointer_curr(&_G(flags)[V016], 0);
					conv_play_curr();
				}
				break;

			case 36:
				_harryShould = 36;
				Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 16, 16);
				break;

			case 37:
				_harryMode = 39;
				_G(flags)[V011] = 2;
				digi_preload_stream_breaks(STREAMS2);
				series_play("102ha09s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS2, "102ha09", 6, 0x100, 3);
				break;

			case 38:
				_harryMode = 40;
				_G(flags)[V011] = 1;
				digi_preload_stream_breaks(STREAMS3);
				series_play("102ha10s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS3, "102ha10", 6, 0x100, 3);
				break;

			case 39:
				digi_unload_stream_breaks(STREAMS2);
				_harryMode = 41;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				break;

			case 40:
				digi_unload_stream_breaks(STREAMS3);
				_harryMode = 41;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				break;

			case 41:
				_wilburChairShould = 65;
				series_unload(_index2);
				_harryMode = 42;
				freeWilburChair();

				digi_preload_stream_breaks(STREAMS4);
				series_play("102ha04s", 0x400, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS4, "102ha04", 6, 0x300, 3);
				break;

			case 42:
				digi_unload_stream_breaks(STREAMS4);
				digi_stop(2);
				_wilburChairMode = 59;
				_wilburChairShould = 58;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_CHAIR_ANIMATION);

				_index3 = series_load("102wi05", -1);
				_index4 = series_load("102ha05", -1);
				_harryMode = 43;
				_harryShould = 43;

				Series::series_play("102ha05", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 0, 4);

				conv_load_and_prepare("conv06", 21);
				conv_export_pointer_curr(&_val4, 0);
				conv_export_value_curr(_G(flags)[V001], 1);
				conv_play_curr();
				_G(flags)[V017] = 1;
				break;

			default:
				break;
			}
			break;

		case 36:
			if (_harryMode == 36) {
				if (!digi_play_state(2)) {
					++_val6;

					if (imath_ranged_rand(4, 6) < _val6) {
						_val6 = 0;

						switch (imath_ranged_rand(1, 4)) {
						case 1:
							digi_play("102_030", 2, 255, -1);
							break;
						case 2:
							digi_play("102_031", 2, 255, -1);
							break;
						case 3:
							digi_play("102_032", 2, 255, -1);
							break;
						case 4:
							digi_play("102_033", 2, 255, -1);
							break;
						default:
							break;
						}
					}
				}

				while ((frame = imath_ranged_rand(16, 21)) == _val3) {
				}

				_val3 = frame;
				Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, frame, frame);

			} else {
				_harryShould = 35;
				Series::series_play("102ha03", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 16, 16);
			}
			break;

		case 43:
			switch (_harryMode) {
			case 20:
				frame = imath_ranged_rand(8, 11);
				Series::series_play("102ha05", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 21:
				frame = imath_ranged_rand(12, 15);
				Series::series_play("102ha05", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 43:
				frame = imath_ranged_rand(4, 7);
				Series::series_play("102ha05", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 44:
				_harryMode = 45;
				Series::series_play("102ha05", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 16, 24);
				conv_resume_curr();
				break;

			case 45:
				Series::series_play("102ha05", 0xa00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 24, 24);
				break;

			case 48:
				_harryMode = 49;
				Series::series_play("102ha05", 0xa00, 2, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 49:
				_harryMode = 50;
				Series::series_play("102ha05", 0xb02, 2, kCHANGE_HARRY_ANIMATION, 30, 0, 100, 0, 0, 8, 8);
				break;

			case 50:
				freeWilburChair();
				series_unload(_index4);
				series_unload(_index3);
				_wilburChairShould = 65;
				_harryMode = 51;
				series_play_with_breaks(PLAY1, "102ha11", 0xa00, kCHANGE_HARRY_ANIMATION, 3, 6, 100, 0, 0);
				break;

			case 51:
				_index1 = series_load("102ha01", -1);
				series_play_with_breaks(PLAY2, "102ha06", 0xa00, 25, 3, 6, 100, 0, 0);
				hotspot_set_active("harry", true);
				hotspot_set_active("harry ", false);

				_G(flags)[V012] = 0;
				if (!_flag1)
					player_set_commands_allowed(true);
				break;
			}
			break;

		case 54:
			switch (_harryMode) {
			case 46:
				terminateMachineAndNull(_series1);
				terminateMachineAndNull(_series1s);
				_harryMode = 47;
				series_play_with_breaks(PLAY2, "102ha06", 0xa00, kCHANGE_HARRY_ANIMATION, 3, 6, 100, 0, 0);
				hotspot_set_active("harry", true);
				break;

			case 47:
				_series1s = series_play("102seats", 0xa01, 0, -1, 100, -1, 100, 0, 0, 0, 0);
				_index1 = series_load("102ha01", -1);
				_harryMode = 19;
				_harryShould = 19;

				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				player_set_commands_allowed(true);

				if (_G(flags)[V015] && !_G(flags)[V013])
					queuePlay("102h202");

				_G(flags)[V012] = 0;
				++_G(flags)[V015];
				break;

			case 53:
				_harryMode = 54;
				_index5 = series_load("102ha07", -1);
				Series::series_play("102ha07", 0xf00, 0, kCHANGE_HARRY_ANIMATION, 10, 0, 100, 0, 0, 0, 7);

				if (_G(flags)[V013]) {
					_play1 = "102h204";
					_harryMode = 56;
					_G(flags)[V014] = 1;
				} else {
					_harryMode = 55;
					_play1 = _G(flags)[V015] ? "102h203" : "102h201";
				}
				break;

			case 54:
				Series::series_play("102ha07", 0xf00, 0, kCHANGE_HARRY_ANIMATION, 30, 0, 100, 0, 0, 23, 23);

				if (_play1) {
					digi_play(_play1, 1, 255, 10);
					_play1 = nullptr;
				}
				break;

			case 55:
				_harryShould = 55;
				Series::series_play("102ha07", 0xf00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 30, 30);
				break;

			case 56:
				_harryShould = 56;
				Series::series_play("102ha07", 0xf00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 9, 10);
				break;

			case 57:
				series_unload(_index5);
				_harryMode = 46;
				series_play("102ha08s", 0xf01, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS5, "102ha08", 6, 0xf00, 3);
				break;

			default:
				break;
			}
			break;

		case 55:
			if (_harryMode == 55) {
				_series3 = series_play("102ha07", 0xf00, 4, -1, 6, -1, 100, 0, 0, 30, 33);
				_series6 = series_play("102ha07s", 0xf01, 4, -1, 6, -1, 100, 0, 0, 30, 33);

				if (_play1) {
					digi_play(_play1, 1, 255, 10);
					_play1 = nullptr;
					kernel_timing_trigger(80, 16);
				}
			} else {
				terminateMachineAndNull(_series3);
				terminateMachineAndNull(_series6);
				_series3 = nullptr;
				_harryShould = 54;
				Series::series_play("102ha07", 0xf00, 2, 3, 6, 0, 100, 0, 0, 30, 30);
			}
			break;

		case 56:
			if (_harryMode == 56) {
				_series3 = series_play("102ha07", 0xf00, 4, -1, 4, -1, 100, 0, 0, 11, 19);
				_series6 = series_play("102ha07s", 0xf01, 4, -1, 4, -1, 100, 0, 0, 11, 19);

				if (_play1) {
					digi_play(_play1, 1, 255, 15);
					_play1 = nullptr;
				}
			} else {
				terminateMachineAndNull(_series3);
				terminateMachineAndNull(_series6);
				_series3 = nullptr;
				_harryShould = 54;
				Series::series_play("102ha07", 0xf00, 0, kCHANGE_HARRY_ANIMATION, 6, 0, 100, 0, 0, 20, 21);
			}
			break;

		default:
			break;
		}
		break;

	case kCHANGE_WILBUR_CHAIR_ANIMATION:
		switch (_wilburChairMode) {
		case 59:
			switch (_wilburChairShould) {
			case 58:
				_wilburChair.play("102wi05", 0xb00, 0, kCHANGE_WILBUR_CHAIR_ANIMATION, 10, -1, 100, 0, 0, 0, 0);
				break;

			case 60:
				freeWilburChair();
				frame = imath_ranged_rand(1, 5);
				Series::series_play("102wi05", 0xb00, 0, kCHANGE_WILBUR_CHAIR_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 63:
				freeWilburChair();
				_wilburChairMode = 64;
				_wilburChairShould = 58;
				series_play_with_breaks(PLAY3, "102wi05", 0xb00, kCHANGE_WILBUR_CHAIR_ANIMATION, 3, 6, 100, 0, 0);
				break;

			default:
				freeWilburChair();
				break;
			}
			break;

		case 62:
			switch (_wilburChairShould) {
			case 58:
				_wilburChair.play("102wi03", 0xb00, 0, -1, 600, -1, 100, 0, 0, 2, 2);
				break;

			case 60:
				freeWilburChair();
				frame = imath_ranged_rand(0, 5);
				Series::series_play("102wi03", 0xb00, 0, kCHANGE_WILBUR_CHAIR_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 61:
				freeWilburChair();
				_wilburChairShould = 60;
				Series::series_play("102wi03", 0xb00, 1, kCHANGE_WILBUR_CHAIR_ANIMATION, 6, 0, 100, 0, 0, 6, 9);
				break;

			default:
				freeWilburChair();
				break;
			}
			break;

		case 64:
			_wilburChairMode = 59;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_CHAIR_ANIMATION);
			break;

		default:
			break;
		}
		break;

	case 6:
		series_play("102tv01", 0, 4, -1, 5, -1, 100, 0, 0, 1, 4);
		break;

	case 8:
		_series1s = series_play("102seats", 0xa01, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		break;

	case 9:
		_harryMode = 44;
		break;

	case 10:
		_harryMode = 57;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		break;

	case 11:
		_series7 = series_play("102ha02s", 0xf01, 0, -1, 600, -1, 100, 0, 0, 40, 40);
		break;

	case 12:
		series_set_frame_rate(_stream1, 30000);
		ws_walk(324, 341, 0, 13, 10, 1);
		break;

	case 13:
		terminateMachineAndNull(_series7);
		series_play("102ha02s", 0xf01, 0, -1, 6, 0, 100, 0, 0, 42, -1);
		series_set_frame_rate(_stream1, 6);
		ws_OverrideCrunchTime(_stream1);
		ws_demand_location(321, 343, 4);
		ws_hide_walker();
		break;

	case 14:
		ws_hide_walker();
		break;

	case 15:
		_harryMode = 54;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1001);
		break;

	case 16:
		ws_walk(318, 343, 0, -1, 2, 1);
		break;

	case 17:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1001);
		break;

	case 18:
		_val8 = 1;
		_val9 = 0;
		_harryMode = player_commands_allowed() && !digi_play_state(1) ? 26 : 19;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		break;

	case 19:
		player_set_commands_allowed(true);
		break;

	case 21:
		if (_G(flags)[V013]) {
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1001);
		} else {
			player_set_commands_allowed(_flag1);
		}
		break;

	case 23:
		_harryMode = 36;
		player_set_commands_allowed(false);

		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("102h200a", 1, 255, 24);
			break;
		case 2:
			digi_play("102h200b", 1, 255, 24);
			break;
		case 3:
			digi_play("102h200c", 1, 255, 24);
			break;
		case 4:
			digi_play("102h200d", 1, 255, 24);
			break;
		default:
			break;
		}
		break;

	case 24:
		player_set_commands_allowed(true);
		_harryMode = 35;
		break;

	case 25:
		_series1s = series_play("102seats", 0xa01, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_harryShould = 19;
		_harryMode = 19;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		conv_resume_curr();
		break;

	case 26:
		_harryMode = 19;
		_val10 = timer_read_60() + 600;

		if (_trigger != -1) {
			_G(kernel).trigger_mode = _triggerMode;
			kernel_trigger_dispatch_now(_trigger);
			_G(kernel).trigger_mode = oldMode;
			_trigger = -1;
		}
		break;

	case 28:
		player_set_commands_allowed(true);
		break;

	case 29:
		if (_G(flags)[V012] != 2)
			player_set_commands_allowed(true);
		_harryMode = 19;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		break;

	case 1037:
		if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE)
			freshen();
		else
			kernel_timing_trigger(60, 1037);
		break;

	case kWILBUR_SPEECH_STARTED:
		break;

	case 10014:
		_G(kernel).continue_handling_trigger = true;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			digi_unload("102_038");
			ws_demand_location(628, 325);
			ws_demand_facing(9);
			_G(wilbur_should) = 2;
			ws_walk(435, 329, 0, kCHANGE_WILBUR_ANIMATION, 9);
			break;

		case 2:
			if (_G(flags)[V012] == 2) {
				_G(wilbur_should) = 3;
				wilbur_speech("102w003", kCHANGE_WILBUR_ANIMATION);
			} else if (_G(flags)[V012] == 0) {
				_G(wilbur_should) = 5;

				if (_G(flags)[V018]) {
					wilbur_speech("102w002", kCHANGE_WILBUR_ANIMATION);
				} else {
					queuePlay("102h001", kCHANGE_WILBUR_ANIMATION);
				}
			}
			break;

		case 3:
			wilbur_speech("102w004", kCHANGE_WILBUR_ANIMATION);
			_G(wilbur_should) = 4;
			break;

		case 4:
			_val16 = 0;
			player_set_commands_allowed(true);
			break;

		case 5:
			if (_G(flags)[V018]) {
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					queuePlay("102h002a");
					break;
				case 2:
					queuePlay("102h002b");
					break;
				case 3:
					queuePlay("102h002c");
					break;
				default:
					break;
				}
			} else {
				_harryMode = 19;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				wilbur_speech("102w001", 28);
				_G(flags)[V018] = 1;
			}
			break;

		case 71:
			ws_hide_walker();
			_G(wilbur_should) = 72;
			digi_preload_stream_breaks(STREAMS6);
			series_play("102wi07s", 0xb00, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS6, "102wi07", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 72:
			digi_unload_stream_breaks(STREAMS6);
			ws_unhide_walker();
			wilbur_speech("102w011", 28);
			break;

		case 73:
			ws_hide_walker();
			_G(wilbur_should) = 74;
			digi_preload_stream_breaks(STREAMS7);
			series_play("102wi06s", 0xb00, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS7, "102wi06", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 74:
			digi_unload_stream_breaks(STREAMS7);
			ws_unhide_walker();
			wilbur_speech("102w011", 28);
			break;

		case 75:
			ws_hide_walker();
			_G(wilbur_should) = 76;
			digi_preload_stream_breaks(STREAMS6);
			series_play("102wi11s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS6, "102wi11", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 76:
			digi_unload_stream_breaks(STREAMS6);
			ws_unhide_walker();
			wilbur_speech("102w011", 28);
			break;

		case 77:
			ws_hide_walker();
			_G(wilbur_should) = 78;
			digi_preload_stream_breaks(STREAMS7);
			series_play("102wi10s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS7, "102wi10", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 78:
			digi_unload_stream_breaks(STREAMS7);
			ws_unhide_walker();
			wilbur_speech("102w011", 28);
			break;

		case 79:
			ws_hide_walker();
			terminateMachineAndNull(_laz1);
			terminateMachineAndNull(_laz2);
			_G(flags)[V019] = 1;
			_G(wilbur_should) = 80;

			if (!_G(flags)[V020])
				digi_preload("102w008");

			series_play_with_breaks(PLAY4, "102wi09", 0x100, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 80:
			ws_unhide_walker();

			if (!_G(flags)[V020]) {
				digi_play("102w008", 1, 255, 81);
				_G(flags)[V020] = 1;
			}

			player_set_commands_allowed(true);
			setupLaz();
			setupWax();
			break;

		case 81:
			digi_unload("102w008");
			break;

		case 82:
			ws_hide_walker();
			terminateMachineAndNull(_laz1);
			terminateMachineAndNull(_laz2);
			_G(flags)[V019] = 0;
			_G(wilbur_should) = 83;
			series_play_with_breaks(PLAY4, "102wi08", 0x100, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 83:
			ws_unhide_walker();
			setupLaz();
			setupWax();
			player_set_commands_allowed(true);
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

void Room102::pre_parser() {
	if (player_said("main street") && !player_said_any("exit", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	if (_G(flags)[V012] == 1) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatch_now(23);
		_G(player).need_to_walk = false;
		_G(kernel).trigger_mode = KT_PARSE;
		g_vars->getInterface()->freshen_sentence();
	} else if (player_said("talk to")) {
		player_hotspot_walk_override(192, 327, 2, -1);
	}
}

void Room102::parser() {
	bool lookFlag = player_said("look") || player_said("look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv04")) {
		conv04();
	} else if (player_said("conv05")) {
		conv05();
	} else if (player_said("conv06")) {
		conv06();
	} else if (player_said("talk to")) {
		conv_load_and_prepare("conv04", 19, false);
		conv_export_value_curr(_G(flags)[V017], 0);
		conv_export_value_curr(_G(flags)[V016], 1);
		conv_play_curr();
	} else if (inv_player_has(_G(player).verb) && player_said("main street")) {
		wilbur_speech("102w015");
	} else if (player_said("exit") || player_said("gear", "main_street")) {
		player_set_commands_allowed(false);
		digi_preload("102_038");
		digi_play("102_038", 2, 255, 17);
	} else {
		if (player_said("TAKE", "DOORWAY"))
			wilbur_speech("102W003");

		if (player_said("USE", "GO", "DOORWAY")) {
			_G(wilbur_should) = 1;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}

		if (player_said("USE", "GO", "FIRE ESCAPE")) {
			wilbur_speech("102W005");
			kernel_trigger_dispatch_now(1003);
		}

		if (player_said("TAKE", "LAZY SUSAN")) {
			flagAction("102w007", "102h004");
		} else if (player_said("GEAR", "LAZY SUSAN")) {
			if (_G(flags)[V012]) {
				player_set_commands_allowed(false);
				_G(wilbur_should) = _G(flags)[V019] ? 82 : 79;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				queuePlay("102h005");
			}
		} else if (player_said("GEAR", "harry") || player_said("take", "harry")) {
			wilbur_speech("102w015");
		} else if (player_said("take", "hair wax") || player_said("take", "hair wax ")) {
			if (_G(flags)[V012]) {
				player_set_commands_allowed(false);
				_G(wilbur_should) = player_said("hair wax") ? 71 : 75;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				queuePlay("102h007");
			}
		} else if (player_said("gear", "hair wax") || player_said("gear", "hair wax ")) {
			flagAction("102w012", "102h007");
		} else if (player_said("take", "television")) {
			flagAction("102w018", "102h012");
		} else if (player_said("gear", "television")) {
			flagAction("102w019", "102h013");
		} else if (player_said("take", "hair wax  ") || player_said("take", "hair wax   ")) {
			if (_G(flags)[V012]) {
				player_set_commands_allowed(false);
				_G(wilbur_should) = player_said("hair wax  ") ? 73 : 77;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				queuePlay("102h007");
			}
		} else if (player_said("gear", "hair wax  ") || player_said("gear", "hair wax   ")) {
			flagAction("102w012", "102h007");
		} else if (player_said("take", "back room") || player_said("gear", "back room")) {
			wilbur_speech("102w015");
		} else if (player_said("enter", "back room")) {
			if (_G(flags)[V012]) {
				switch (_G(kernel).trigger) {
				case -1:
					player_set_commands_allowed(false);
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play("102h015", 1, 255, 1);
					break;
				case 1:
					freshen();
					break;
				default:
					break;
				}
			} else {
				queuePlay("102h014");
			}
		} else if (player_said("take", "barber's chair")) {
			flagAction("102w021", "102h016");
		} else if (player_said("gear", "barber's chair")) {
			flagAction("102w022", "102h017");
		} else if (player_said("take", "fish")) {
			flagAction("102w025", "102h019");
		} else if (player_said("gear", "fish")) {
			flagAction("102w026", "102h020");
		} else if (player_said("gear", "bottles")) {
			wilbur_speech("102w029");
		} else if (player_said("take", "bottles")) {
			if (_G(flags)[V012]) {
				switch (_G(kernel).trigger) {
				case -1:
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play("102h023", 1, 255, 1);
					player_set_commands_allowed(false);
					break;
				case 1:
					_G(kernel).trigger_mode = KT_PARSE;
					wilbur_speech("102w028");
					break;
				default:
					break;
				}
			} else {
				player_set_commands_allowed(false);
			}
		} else if (player_said("take", "towels")) {
			wilbur_speech("102w2029");
		} else if (player_said("gear", "towels")) {
			flagAction("102w032", "102h025");
		} else if (player_said("take", "mirror")) {
			wilbur_speech("102w015");
		} else if (player_said("gear", "mirror")) {
			flagAction("102w034", "102h027");
		} else if (player_said("take", "sink")) {
			wilbur_speech("102w037");
		} else if (player_said("gear", "sink")) {
			flagAction("102w038", "102h029");
		} else if (player_said("take", "stove")) {
			wilbur_speech("102w040");
		} else if (player_said("gear", "stove")) {
			flagAction("102w041", "102h031");
		} else if (player_said("harry") && inv_player_has(_G(player).verb) && !_G(flags)[V012]) {
			queuePlay(imath_ranged_rand(1, 2) == 1 ? "102h009y" : "102h009z");
		} else if (lookFlag) {
			if (player_said("Harry")) {
				switch (_G(kernel).trigger) {
				case -1:
					if (!_G(flags)[V012]) {
						_G(kernel).trigger_mode = KT_PARSE;
						wilbur_speech("102w014");
						player_set_commands_allowed(false);
					}
					break;
				case 1:
					queuePlay("102h009", 2, KT_PARSE);
					break;
				case 2:
					kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
					_G(kernel).trigger_mode = KT_PARSE;
					kernel_timing_trigger(120, 3);
					break;
				case 3:
					queuePlay("102h010");
					break;
				default:
					break;
				}
			} else if (player_said("HAIR WAX") || player_said("HAIR WAX ")) {
				flagAction("102w009", "102h006");
			} else if (player_said("HAIR WAX  ") || player_said("HAIR WAX   ")) {
				flagAction("102w013", "102h008");
			} else if (player_said("LAZY SUSAN")) {
				flagAction("102w005", "102h003");
			} else if (player_said("television")) {
				flagAction("102w017", "102h011");
			} else if (player_said("back room")) {
				if (_G(flags)[V012]) {
					switch (_G(kernel).trigger) {
					case -1:
						player_set_commands_allowed(false);
						_G(kernel).trigger_mode = KT_PARSE;
						digi_play("102h015", 1, 255, 1);
						break;
					case 1:
						freshen();
						break;
					default:
						break;
					}
				} else {
					queuePlay("102h014");
				}
			} else if (player_said("barber's chair")) {
				wilbur_speech("102W020");
			} else if (player_said("fish")) {
				flagAction("102w023", "102h018");
			} else if (player_said("bottles")) {
				flagAction("102w027", "102h021");
			} else if (player_said("towels")) {
				flagAction("102w030", "102h024");
			} else if (player_said("mirror")) {
				flagAction("102w033", "102h026");
			} else if (player_said("sink")) {
				flagAction("102w036", "102h028");
			} else if (player_said("stove")) {
				flagAction("102w039", "102h030");
			} else {
				return;
			}
		} else {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room102::flagAction(const char *name1, const char *name2) {
	switch (_G(kernel).trigger) {
	case -1:
		if (_G(flags)[V012]) {
			wilbur_speech(name1);
		} else {
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech(name1, 1);
			player_set_commands_allowed(false);
		}
		break;
	case 1:
		queuePlay(name2);
		break;
	default:
		break;
	}
}


void Room102::setup(int val1, int val2) {
	digi_stop(3);

	if (val2)
		digi_play_loop("102_001", 3, 255, -1);
	if (_val1) {
		digi_unload(getDigi1(_G(flags)[V008]));
		digi_unload(getDigi2(_G(flags)[V008]));
	}

	if (val1) {
		_G(flags)[V008] = val1;
	} else {
		if (++_G(flags)[V008] == 36)
			_G(flags)[V008] = 1;
	}

	_val1 = 1;
	digi_preload(getDigi1(_G(flags)[V008]));
	digi_preload(getDigi2(_G(flags)[V008]));

	if (val2)
		digi_stop(3);

	digi_play_loop(getDigi2(_G(flags)[V008]), 3, 255, -1);
	_val10 = timer_read_60() + digi_ticks_to_play(getDigi2(_G(flags)[V008]));
	_val8 = 0;
}

void Room102::setupWax() {
	if (_G(flags)[V019]) {
		hotspot_set_active("hair wax   ", false);
		hotspot_set_active("hair wax  ", true);
		hotspot_set_active("hair wax ", true);
		hotspot_set_active("hair wax", false);
	} else {
		hotspot_set_active("hair wax   ", true);
		hotspot_set_active("hair wax  ", false);
		hotspot_set_active("hair wax ", false);
		hotspot_set_active("hair wax", true);
	}
}

void Room102::setupLaz() {
	if (_G(flags)[V019]) {
		_laz1 = series_play("102lazys", 512, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_laz2 = series_play("102lazss", 513, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	} else {
		_laz1 = series_play("102lazy2", 512, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_laz2 = series_play("102laz2s", 513, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	}
}

const char *Room102::getDigi1(int num) const {
	static const char *NAMES[34] = {
		"102h0323", "102h032b", "102h032c", "102h032d",
		"102h032e", "102h032f", "102h032g", "102h032h",
		"102h032i", "102h032j", "102h032k", "102h032l",
		"102h032m", "102h0321", "102h032o", "102h032p",
		"102h032q", "102h032r", "102h032s", "102h0324",
		"102h032u", "102h0321", "102h032w", "102h032x",
		"102h032y", "102h032z", "102h032v", "102h0322",
		"102h032a", "102h032t", "102h0325", "102h0326",
		"102h0327", "102h0328" };

	return (num >= 1 && num <= 34) ? NAMES[num - 1] : "102h0329";
}

const char *Room102::getDigi2(int num) const {
	static const char *NAMES[35] = {
		"102_011", "102_004", "102_024", "102_006", "102_008",
		"102_009", "102_013", "102_005", "102_014", "102_023",
		"102_020", "102_011", "102_015", "102_006", "102_007",
		"102_016", "102_014", "102_015", "102_013", "102_011",
		"102_003", "102_012", "102_008", "102_013", "102_021",
		"102_011", "102_013", "102_010", "102_003", "102_005",
		"102_010", "102_011", "102_021", "102_021", "102_012"
	};

	return (num >= 1 && num <= 35) ? NAMES[num - 1] : "102_024";
}

void Room102::freeWilburChair() {
	_wilburChair.terminate();
}

void Room102::queuePlay(const char *filename, int trigger, KernelTriggerType triggerMode) {
	if (_val9) {
		digi_stop(2);
		_val9 = 0;
		_val8 = 1;
	}

	_play1 = filename;
	_harryMode = 20;
	_trigger = trigger;
	_triggerMode = triggerMode;
	player_set_commands_allowed(false);

	term_message("spleen %d %d    %d %d", _harryShould, _harryMode, 19, 20);

	if (_harryShould == 20) {
		term_message("spleen2");
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
	}
}

void Room102::freshen() {
	if (_G(flags)[V012] == 2) {
		player_set_commands_allowed(false);
		_G(flags)[V012] = 0;
		_harryShould = 54;
		_harryMode = 53;

		kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
		g_vars->getInterface()->freshen_sentence();
		Section1::walk();
	}
}

void Room102::conv04() {
	_G(kernel).trigger_mode = KT_PARSE;
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 22) {
		if (who <= 0) {
			_G(kernel).trigger_mode = KT_DAEMON;

			if (node == 1 && entry == 0) {
				_harryMode = 26;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				player_update_info();
				ws_walk(192, 327, 0, -1, 4);
				conv_resume_curr();

			} else if (node == 4 && entry == 0) {
				digi_preload("102_030");
				digi_preload("102_031");
				digi_preload("102_032");
				digi_preload("102_033");
				_harryMode = 28;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
			} else if ((node == 4 && entry == 1) || (node == 4 && entry == 2)) {
				_harryMode = 35;
				conv_resume_curr();
			} else {
				_harryMode = 19;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
				conv_resume_curr();
			}
		} else if (who == 1) {
			if (node == 4 && entry == 1) {
				// Nothing
			} else if (node == 4 && entry == 2) {
				_wilburChairShould = 58;
				conv_resume_curr();
			} else {
				_G(wilbur_should) = 10002;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				conv_resume_curr();
			}
		}
	} else if (sound) {
		if (who <= 0) {
			if ((node == 4 && entry == 1) || (node == 4 && entry == 2)) {
				_harryMode = 36;
				digi_play(sound, 1, 255, 22);
				_G(flags)[V012] = 1;
			} else {
				queuePlay(sound, 22, KT_PARSE);
			}
		} else if (who == 1) {
			player_update_info();

			if (_G(player).walker_visible && _G(player_info).facing != 2) {
				ws_walk(192, 327, 0, 7, 2);
			} else if (node == 4 && entry == 1) {
				digi_preload("04p0502", 102);
				digi_play(sound, 1, 255, 22);
			} else if (node == 4 && entry == 2) {
				_wilburChairShould = 60;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(4);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(sound, 1, 255, 22);
			} else {
				wilbur_speech(sound, 22);
			}
		}
	} else {
		conv_resume_curr();
	}
}

void Room102::conv05() {
	_G(kernel).trigger_mode = KT_PARSE;
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 22) {
		if (who <= 0) {
			if ((node == 1 && entry == 3) || (node == 3 && entry == 9) ||
					(node == 5 && entry == 8) || (node == 2)) {
				_harryMode = 41;
			} else if ((node == 1 && entry == 0) || (node == 3 && entry == 8) ||
					(node == 5 && entry == 7)) {
				_harryMode = _G(flags)[V019] ? 38 : 37;
			} else {
				_harryMode = 35;
			}		
		} else if (who == 1) {
			_wilburChairShould = 58;
		}

		conv_resume_curr();

	} else if (sound) {
		if (who <= 0) {
			_harryMode = 36;
		} else if (who == 1) {
			if ((node == 1 && entry == 1) || (node == 3 && entry == 0) ||
					(node == 3 && entry == 3) || (node == 5 && entry == 3)) {
				_wilburChairShould = 61;				
			} else {
				_wilburChairShould = 60;
			}

			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatch_now(4);
		}

		_G(kernel).trigger_mode = KT_PARSE;
		digi_play(sound, 1, 255, 22);
	} else {
		conv_resume_curr();
	}
}

void Room102::conv06() {
	_G(kernel).trigger_mode = KT_PARSE;
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 22) {
		if (who <= 0) {
			if (node == 2) {
				_harryMode = 19;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(kCHANGE_HARRY_ANIMATION);
			} else if (node == 1 && entry == 4) {
				_G(flags)[V014] = 1;
				_harryMode = 43;
			} else if (node != 1 || entry != 1) {
				_harryMode = 43;
			}

			conv_resume_curr();

		} else if (who == 1) {
			if (node == 1 && entry == 1) {
				_wilburChairShould = 63;
			} else if (node == 2) {
				_harryMode = _G(flags)[V010] == 1 ? 49 : 48;
				_wilburChairShould = 58;
			} else {
				_wilburChairShould = 58;
				conv_resume_curr();
			}
		}
	} else if (sound) {
		if (who <= 0) {
			if (node == 1 && entry == 3) {
				_G(flags)[V010] = 3;

				if (_G(flags)[V011] == 1)
					_G(flags)[V021] = 10034;
			}

			if (node == 2) {
				term_message("gonna_say_sheepdog %d %d", _harryShould, _harryMode);
				queuePlay(sound, 22, KT_PARSE);
				_flag1 = true;
				return;

			} else if (node == 1 && entry == 1) {
				_G(flags)[V010] = 1;
				_G(flags)[V001] -= 8;

				if (_G(flags)[V011] == 1)
					_G(flags)[V021] = 10032;
			} else if (node == 1 && entry == 4) {
				_harryMode = 21;
				_G(flags)[V010] = 2;

				if (_G(flags)[V011] == 1)
					_G(flags)[V021] = 10033;

				_G(flags)[V013] = 1;
				_flag1 = false;
			} else {
				_harryMode = 20;
			}

		} else if (who == 1) {
			if (_wilburChairMode == 64) {
				kernel_timing_trigger(30, 7);
				return;

			} else {
				_wilburChairShould = 60;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(4);
			}
		}

		_G(kernel).trigger_mode = KT_PARSE;
		digi_play(sound, 1, 255, 22);

	} else {
		conv_resume_curr();
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
