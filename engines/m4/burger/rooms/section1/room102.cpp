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
	_series4 = 0;
	_val8 = 0;
	_val9 = 0;
	_val10 = 99999;

	digi_preload("102_001");
	digi_preload("102_002");

	switch (_G(flags)[V012]) {
	case 1:
		_index1 = series_load("102ha01", -1);
		_index2 = series_load("102ha03", -1);
		_val11 = 35;
		_val12 = 35;
		kernel_trigger_dispatch_now(3);

		_val13 = 58;
		_val14 = 62;
		kernel_trigger_dispatch_now(4);

		hotspot_set_active("harry", false);
		setup(5, 0);
		break;

	case 2:
		hotspot_set_active("harry", false);
		hotspot_set_active("harry ", false);

		_series1 = series_play("102seat", 2560, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_series2 = series_play("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		setup(5, 0);
		break;

	default:
		_index1 = series_load("102ha01", -1);
		setup(0, 0);

		_val11 = 19;
		_val12 = 19;
		kernel_trigger_dispatch_now(3);
		++_G(flags)[V015];

		_series2 = series_play("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		hotspot_set_active("harry ", false);
		break;
	}

	_trigger = -1;

	if (_G(game).previous_room == -2) {
		if (_G(flags)[V012] == 1) {
			ws_demand_location(321, 343);
			ws_demand_facing(4);
			ws_hide_walker();
		}
	} else {
		_G(wilbur_should) = 1;
		_val16 = 1;
		digi_preload("102_038");
		digi_play("102_038", 2, 255, gCHANGE_WILBUR_ANIMATION);
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
		ws_unhide_walker(_G(my_walker));
		break;
	case 3:
		switch (_val12) {
		case 19:
			term_message(" mumble or change channel");

			if (timer_read_60() > _val10 &&_val11 == 19 && player_commands_allowed() &&
					!digi_play_state(1) && INTERFACE_VISIBLE) {
				if (_val8) {
					_val11 = 26;
					term_message("change channel");
				} else {
					digi_play(getDigi1(_G(flags)[GLB_TEMP_2]), 2, 255, 18);
					_val9 = 1;
					_val11 = 20;
					term_message("mumble");
				}
			}

			term_message("!");

			switch (_val11) {
			case 19:
				switch (imath_ranged_rand(1, 45)) {
				case 1:
				case 2:
				case 3:
					series_play("102ha01", 2560, 1, 3, 4, 0, 100, 0, 0, 0, 0);
					break;
				case 6:
					_val12 = 30;
					series_play("102ha01", 2560, 2, 3, 10, 0, 100, 0, 0, 19, 19);
					break;
				case 7:
					_val12 = 31;
					series_play("102ha01", 2560, 2, 3, 10, 0, 100, 0, 0, 20, 23);
					break;
				case 8:
					_val12 = 24;
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 1, 1);
					break;
				case 9:
					_val12 = 25;
					series_play("102ha01", 2560, 0, 3, 7, 0, 100, 0, 0, 3, 4);
					break;
				case 10:
					_val12 = 23;
					series_play("102ha01", 2560, 0, 3, 8, 0, 100, 0, 0, 46, 48);
					break;
				default:
					series_play("102ha01", 2560, 0, 3, 4, 0, 100, 0, 0, 0, 0);
					break;
				}
				break;

			case 20:
				_val12 = 20;
				series_play("102ha01", 2560, 0, 3, 4, 0, 100, 0, 0, 24, 24);
				break;

			case 21:
				_val12 = 21;
				series_play("102ha01", 2560, 0, 3, 4, 0, 100, 0, 0, 30, 30);
				break;

			case 26:
				_val11 = 27;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 8, 14);
				break;

			case 27:
				if (player_commands_allowed() && !digi_play_state(1)) {
					setup();
					_val11 = 19;
					_val12 = 26;
					kernel_trigger_dispatch_now(3);
				} else {
					series_play("102ha01", 2560, 0, 3, 60, 0, 100, 0, 0, 14, 14);
				}
				break;

			case 28:
				_val11 = 29;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 8, 14);
				break;

			case 29:
				setup(5);
				_val11 = 32;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 15, 15);
				break;

			case 32:
				series_unload(_index1);
				terminateMachineAndNull(_series2);
				_val11 = 33;
				series_play("102ha02s", 3841, 0, 11, 6, 0, 100, 0, 0, 0, 41);
				digi_preload_stream_breaks(STREAMS1);
				series_stream_with_breaks(STREAMS1, "102ha02", 6, 3840, 3);
				break;

			case 33:
				digi_unload_stream_breaks(STREAMS1);
				_val14 = 62;
				_val13 = 58;
				kernel_trigger_dispatch_now(4);
				digi_stop(2);
				_index2 = series_load("102ha03", -1);
				_val11 = 34;
				series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 0, 4);
				series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 34:
				_val11 = 35;
				_val12 = 35;
				kernel_trigger_dispatch_now(3);
				hotspot_set_active("harry", false);
				hotspot_set_active("harry ", true);
				conv_resume_curr();
				break;

			default:
				break;
			}
			break;

		case 20:
			if (_val11 == 20) {
				if (_play1) {
					digi_play(_play1, 1, 255, 26);
					_play1 = nullptr;
				}
				if (_series3)
					terminateMachineAndNull(_series3);

				_series3 = series_play("102ha01", 2560, 4, -1, 5, -1, 100, 0, 0, 24, 29);
			} else {
				terminateMachineAndNull(_series3);
				_series3 = nullptr;
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 4, 0, 100, 0, 0, 24, 24);
			}
			break;

		case 21:
			if (_val11 == 21) {
				if (_play1) {
					digi_play(_play1, 1, 255, 26);
					_play1 = nullptr;
				}

				_series3 = series_play("102ha01", 2560, 4, -1, 5, -1, 100, 0, 0, 31, 42);
			} else {
				terminateMachineAndNull(_series3);
				_series3 = nullptr;
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 4, 0, 100, 0, 0, 30, 30);
			}
			break;

		case 22:
			_val12 = 23;
			series_play("102ha01", 2560, 2, 3, 6, 0, 100, 0, 0, 44, 48);
			break;

		case 23:
			_val12 = 19;
			series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 44, 46);
			break;

		case 24:
			if (_val11 == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_val12 = 19;
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 0, 0);
				} else {
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 1, 1);
				}
			} else {
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 25:
			if (_val11 == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_val12 = 19;
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 5, 7);
				} else {
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 1, 1);
				}
			} else {
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 5, 7);
			}
			break;

		case 26:
			if (_val11 == 19) {
				if (imath_ranged_rand(1, 17) == 1) {
					_val12 = 19;
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 15, 15);
				} else {
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 14, 14);
				}
			} else {
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 15, 15);
			}
			break;

		case 30:
			if (_val11 == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_val12 = 19;
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 16, 19);
				} else {
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 16, 16);
				}
			} else {
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 16, 19);
			}
			break;

		case 31:
			if (_val11 == 19) {
				if (imath_ranged_rand(1, 15) == 1) {
					_val12 = 19;
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 20, 23);
				} else {
					series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 20, 20);
				}
			} else {
				_val12 = 19;
				series_play("102ha01", 2560, 0, 3, 6, 0, 100, 0, 0, 20, 23);
			}
			break;

		case 35:
			switch (_val11) {
			case 35:
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					if (!digi_play_state(2))
						digi_play("102_030", 2, 255, -1);

					series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 4, 6);
					series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 4, 6);
					break;

				case 2:
					if (!digi_play_state(2))
						digi_play("102_031", 2, 255, -1);

					series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 7, 8);
					series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 7, 8);
					break;

				case 3:
					if (!digi_play_state(2))
						digi_play("102_032", 2, 255, -1);

					series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 9, 11);
					series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 9, 11);
					break;

				case 4:
					if (!digi_play_state(2))
						digi_play("102_033", 2, 255, -1);

					series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 12, 15);
					series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 12, 15);
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
				_val12 = 36;
				series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 16, 16);
				series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 16, 16);
				break;

			case 37:
				_val11 = 39;
				_G(flags)[GLB_TEMP_5] = 2;
				digi_preload_stream_breaks(STREAMS2);
				series_play("102ha09s", 257, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS2, "102ha09", 6, 256, 3);
				break;

			case 38:
				_val11 = 40;
				_G(flags)[GLB_TEMP_5] = 1;
				digi_preload_stream_breaks(STREAMS3);
				series_play("102ha10s", 257, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS3, "102ha10", 6, 256, 3);
				break;

			case 39:
				digi_unload_stream_breaks(STREAMS2);
				_val11 = 41;
				kernel_trigger_dispatch_now(3);
				break;

			case 40:
				digi_unload_stream_breaks(STREAMS3);
				_val11 = 41;
				kernel_trigger_dispatch_now(3);
				break;

			case 41:
				_val13 = 65;
				series_unload(_index2);
				_val11 = 42;
				sub1();

				digi_preload_stream_breaks(STREAMS4);
				series_play("102ha04s", 1024, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS4, "102ha04", 6, 768, 3);
				break;

			case 42:
				digi_unload_stream_breaks(STREAMS4);
				digi_stop(2);
				_val14 = 59;
				_val13 = 58;
				kernel_trigger_dispatch_now(4);

				_index3 = series_load("102wi05", -1);
				_index4 = series_load("102ha05", -1);
				_val11 = 43;
				_val12 = 43;

				series_play("102ha05", 2560, 0, 3, 6, 0, 100, 0, 0, 0, 4);
				series_play("102ha05s", 2561, 0, -1, 6, 0, 100, 0, 0, 0, 4);

				conv_load_and_prepare("conv06", 21, false);
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
			if (_val11 == 36) {
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

				_val13 = frame;
				series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, frame, frame);
				series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, frame, frame);

			} else {
				_val13 = 35;
				series_play("102ha03", 2560, 0, 3, 6, 0, 100, 0, 0, 16, 16);
				series_play("102ha03s", 2561, 0, -1, 6, 0, 100, 0, 0, 16, 16);
			}
			break;

		case 43:
			switch (_val11) {
			case 20:
				frame = imath_ranged_rand(8, 11);
				series_play("102ha05", 2560, 0, 3, 4, 0, 100, 0, 0, frame, frame);
				series_play("102ha05s", 2561, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 21:
				frame = imath_ranged_rand(12, 15);
				series_play("102ha05", 2560, 0, 3, 4, 0, 100, 0, 0, frame, frame);
				series_play("102ha05s", 2561, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 43:
				frame = imath_ranged_rand(4, 7);
				series_play("102ha05", 2560, 0, 3, 4, 0, 100, 0, 0, frame, frame);
				series_play("102ha05s", 2561, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 44:
				_val11 = 45;
				series_play("102ha05", 2560, 0, 3, 6, 0, 100, 0, 0, 16, 24);
				series_play("102ha05s", 2561, 0, -1, 6, 0, 100, 0, 0, 16, 24);
				break;

			case 45:
				series_play("102ha05", 2560, 0, 3, 6, 0, 100, 0, 0, 24, 24);
				series_play("102ha05s", 2561, 0, -1, 6, 0, 100, 0, 0, 24, 24);
				break;

			case 48:
				_val11 = 49;
				series_play("102ha05", 2560, 2, 3, 6, 0, 100, 0, 0, 0, 3);
				series_play("102ha05s", 2561, 2, -1, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 49:
				_val11 = 50;
				series_play("102ha05", 2818, 2, 3, 30, 0, 100, 0, 0, 8, 8);
				series_play("102ha05s", 2818, 2, -1, 30, 0, 100, 0, 0, 8, 8);
				break;

			case 50:
				sub1();
				series_unload(_index4);
				series_unload(_index3);
				_val13 = 65;
				_val11 = 51;
				series_play_with_breaks(PLAY1, "102ha11", 2560, 3, 3, 6, 100, 0, 0);
				break;

			case 51:
				_index1 = series_load("102ha01", -1);
				series_play_with_breaks(PLAY2, "102ha06", 2560, 25, 3, 6, 100, 0, 0);
				hotspot_set_active("harry", true);
				hotspot_set_active("harry ", false);

				_G(flags)[V012] = 0;
				if (!_flag1)
					player_set_commands_allowed(true);
				break;
			}
			break;

		case 54:
			switch (_val11) {
			case 46:
				terminateMachineAndNull(_series1);
				terminateMachineAndNull(_series2);
				_val11 = 47;
				series_play_with_breaks(PLAY2, "102ha06", 2560, 3, 3, 6, 100, 0, 0);
				hotspot_set_active("harry", true);
				break;

			case 47:
				_series2 = series_play("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
				_index1 = series_load("102ha01", -1);
				_val11 = 19;
				_val12 = 19;

				kernel_trigger_dispatch_now(3);
				player_set_commands_allowed(true);

				if (_G(flags)[V015] && !_G(flags)[V013])
					queuePlay("102h202");

				_G(flags)[V012] = 0;
				++_G(flags)[V015];
				break;

			case 53:
				_val11 = 54;
				_index5 = series_load("102ha07", -1);
				series_play("102ha07", 3840, 0, 3, 10, 0, 100, 0, 0, 0, 7);
				series_play("102ha07s", 3841, 0, -1, 10, 0, 100, 0, 0, 0, 7);

				if (_G(flags)[V013]) {
					_play1 = "102h204";
					_val11 = 56;
					_G(flags)[ROOM101_FLAG1] = 1;
				} else {
					_val11 = 55;
					_play1 = _G(flags)[V015] ? "102h203" : "102h201";
				}
				break;

			case 54:
				series_play("102ha07", 3840, 0, 3, 30, 0, 100, 0, 0, 23, 23);
				series_play("102ha07s", 3841, 0, -1, 30, 0, 100, 0, 0, 23, 23);

				if (_play1) {
					digi_play(_play1, 1, 255, 10);
					_play1 = nullptr;
				}
				break;

			case 55:
				_val12 = 55;
				series_play("102ha07", 3840, 0, 3, 6, 0, 100, 0, 0, 30, 30);
				series_play("102ha07s", 3841, 0, -1, 6, 0, 100, 0, 0, 30, 30);
				break;

			case 56:
				_val12 = 56;
				series_play("102ha07", 3840, 0, 3, 6, 0, 100, 0, 0, 9, 10);
				series_play("102ha07s", 3841, 0, -1, 6, 0, 100, 0, 0, 9, 10);
				break;

			case 57:
				series_unload(_index5);
				_val11 = 46;
				series_play("102ha08s", 3841, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				series_stream_with_breaks(STREAMS5, "102ha08", 6, 3840, 3);
				break;

			default:
				break;
			}
			break;

		case 55:
			if (_val11 == 55) {
				_series3 = series_play("102ha07", 3840, 4, -1, 6, -1, 100, 0, 0, 30, 33);
				_series6 = series_play("102ha07s", 3841, 4, -1, 6, -1, 100, 0, 0, 30, 33);

				if (_play1) {
					digi_play(_play1, 1, 255, 10);
					_play1 = nullptr;
					kernel_timing_trigger(80, 16);
				}
			} else {
				terminateMachineAndNull(_series3);
				terminateMachineAndNull(_series6);
				_series3 = nullptr;
				_val12 = 54;
				series_play("102ha07", 3840, 2, 3, 6, 0, 100, 0, 0, 30, 30);
				series_play("102ha07s", 3841, 2, -1, 6, 0, 100, 0, 0, 30, 30);
			}
			break;

		case 56:
			if (_val11 == 56) {
				_series3 = series_play("102ha07", 3840, 4, -1, 4, -1, 100, 0, 0, 11, 19);
				_series6 = series_play("102ha07s", 3841, 4, -1, 4, -1, 100, 0, 0, 11, 19);

				if (_play1) {
					digi_play(_play1, 1, 255, 15);
					_play1 = nullptr;
				}
			} else {
				terminateMachineAndNull(_series3);
				terminateMachineAndNull(_series6);
				_series3 = nullptr;
				_val12 = 54;
				series_play("102ha07", 3840, 0, 3, 6, 0, 100, 0, 0, 20, 21);
				series_play("102ha07s", 3841, 0, -1, 6, 0, 100, 0, 0, 20, 21);
			}
			break;

		default:
			break;
		}
		break;

	case 4:
		switch (_val14) {
		case 59:
			switch (_val13) {
			case 58:
				_series4 = series_play("102wi05", 2816, 0, 4, 10, -1, 100, 0, 0, 0, 0);
				_series5 = series_play("102wi05s", 2817, 0, -1, 10, -1, 100, 0, 0, 0, 0);
				break;

			case 60:
				frame = imath_ranged_rand(1, 5);
				series_play("102wi05", 2816, 0, 4, 4, 0, 100, 0, 0, frame, frame);
				series_play("102wi05s", 2817, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 63:
				sub1();
				_val1 = 64;
				_val13 = 58;
				series_play_with_breaks(PLAY3, "102wi05", 2816, 4, 3, 6, 100, 0, 0);
				break;

			default:
				sub1();
				break;
			}
			break;

		case 62:
			switch (_val13) {
			case 58:
				_series4 = series_play("102wi03", 2816, 0, -1, 600, -1, 100, 0, 0, 2, 2);
				_series5 = series_play("102wi03s", 2816, 0, -1, 600, -1, 100, 0, 0, 2, 2);
				break;

			case 60:
				sub1();
				frame = imath_ranged_rand(0, 5);
				_series4 = series_play("102wi03", 2816, 0, 4, 4, 0, 100, 0, 0, frame, frame);
				_series5 = series_play("102wi03s", 2816, 0, 4, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 61:
				sub1();
				_val13 = 60;
				series_play("102wi03", 2816, 1, 4, 6, 0, 100, 0, 0, 6, 9);
				series_play("102wi03s", 2816, 1, -1, 6, 0, 100, 0, 0, 6, 9);
				break;

			default:
				sub1();
				break;
			}
			break;

		case 64:
			_val14 = 59;
			kernel_trigger_dispatch_now(4);
			break;

		default:
			break;
		}
		break;

	case 6:
		series_play("102tv01", 0, 4, -1, 5, -1, 100, 0, 0, 1, 4);
		break;

	case 8:
		_series2 = series_play("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		break;

	case 9:
		_val11 = 44;
		break;

	case 10:
		_val11 = 57;
		kernel_trigger_dispatch_now(3);
		break;

	case 11:
		_series7 = series_play("102ha02s", 3841, 0, -1, 600, -1, 100, 0, 0, 40, 40);
		break;

	case 12:
		series_set_frame_rate(_stream1, 30000);
		ws_walk(_G(my_walker), 324, 341, 0, 13, 10, 1);
		break;

	case 13:
		terminateMachineAndNull(_series7);
		series_play("102ha02s", 3841, 0, -1, 6, 0, 100, 0, 0, 42, -1);
		series_set_frame_rate(_stream1, 6);
		ws_OverrideCrunchTime(_stream1);
		ws_demand_location(_G(my_walker), 321, 343);
		ws_demand_facing(_G(my_walker), 4);
		ws_hide_walker(_G(my_walker));
		break;

	case 14:
		ws_hide_walker(_G(my_walker));
		break;

	case 15:
		_val11 = 54;
		kernel_trigger_dispatch_now(3);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1001);
		break;

	case 16:
		ws_walk(_G(my_walker), 318, 343, 0, -1, 2, 1);
		break;

	case 17:
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1001);
		break;

	case 18:
		_val8 = 1;
		_val9 = 0;
		_val11 = player_commands_allowed() && !digi_play_state(1) ? 26 : 19;
		kernel_trigger_dispatch_now(3);
		break;

	case 19:
		player_set_commands_allowed(true);
		break;

	case 21:
		if (_G(flags)[V013]) {
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1001);
		} else {
			player_set_commands_allowed(_flag1);
		}
		break;

	case 23:
		_val11 = 36;
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
		_val11 = 35;
		break;

	case 25:
		_series2 = series_play("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_val12 = 19;
		_val11 = 19;
		kernel_trigger_dispatch_now(3);
		conv_resume_curr();
		break;

	case 26:
		_val11 = 19;
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
		if (_G(flags)[V012] == 2)
			player_set_commands_allowed(true);
		_val11 = 19;
		kernel_trigger_dispatch_now(3);
		break;

	case 1037:
		if (player_commands_allowed() && _G(roomVal2) && INTERFACE_VISIBLE)
			freshen();
		else
			kernel_timing_trigger(60, 1037);
		break;

	case 10013:
		break;

	case 10014:
		_G(kernel).continue_handling_trigger = true;
		break;

	case gCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			digi_unload("102_038");
			ws_demand_location(628, 325);
			ws_demand_facing(9);
			_G(wilbur_should) = 2;
			ws_walk(435, 329, 0, gCHANGE_WILBUR_ANIMATION, 9, 1);
			break;

		case 2:
			if (_G(flags)[V012] == 2) {
				_G(wilbur_should) = 3;
				_G(walker).wilbur_speech("102w003", gCHANGE_WILBUR_ANIMATION);
			} else if (_G(flags)[V012] == 0) {
				_G(wilbur_should) = 5;

				if (_G(flags)[V018]) {
					_G(walker).wilbur_speech("102w002", gCHANGE_WILBUR_ANIMATION);
				} else {
					queuePlay("102h001", gCHANGE_WILBUR_ANIMATION);
				}
			}
			break;

		case 3:
			_G(walker).wilbur_speech("102w004", gCHANGE_WILBUR_ANIMATION);
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
				_val11 = 19;
				kernel_trigger_dispatch_now(3);
				_G(walker).wilbur_speech("102w001", 28);
				_G(flags)[V018] = 1;
			}
			break;

		case 71:
			ws_hide_walker();
			_G(wilbur_should) = 72;
			digi_preload_stream_breaks(STREAMS6);
			series_play("102wi07s", 2816, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS6, "102wi07", 6, 256, gCHANGE_WILBUR_ANIMATION);
			break;

		case 72:
			digi_unload_stream_breaks(STREAMS6);
			ws_unhide_walker();
			_G(walker).wilbur_speech("102w011", 28);
			break;

		case 73:
			ws_hide_walker();
			_G(wilbur_should) = 74;
			digi_preload_stream_breaks(STREAMS7);
			series_play("102wi06s", 2816, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS7, "102wi06", 6, 256, gCHANGE_WILBUR_ANIMATION);
			break;

		case 74:
			digi_unload_stream_breaks(STREAMS7);
			ws_unhide_walker();
			_G(walker).wilbur_speech("102w011", 28);
			break;

		case 75:
			ws_hide_walker();
			_G(wilbur_should) = 76;
			digi_preload_stream_breaks(STREAMS6);
			series_play("102wi11s", 257, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS6, "102wi11", 6, 256, gCHANGE_WILBUR_ANIMATION);
			break;

		case 76:
			digi_unload_stream_breaks(STREAMS6);
			ws_unhide_walker();
			_G(walker).wilbur_speech("102w011", 28);
			break;

		case 77:
			ws_hide_walker();
			_G(wilbur_should) = 78;
			digi_preload_stream_breaks(STREAMS7);
			series_play("102wi10s", 257, 0, -1, 6, 0, 100, 0, 0, 0, -1);
			series_stream_with_breaks(STREAMS7, "102wi10", 6, 256, gCHANGE_WILBUR_ANIMATION);
			break;

		case 78:
			digi_unload_stream_breaks(STREAMS7);
			ws_unhide_walker();
			_G(walker).wilbur_speech("102w011", 28);
			break;

		case 79:
			ws_hide_walker();
			terminateMachineAndNull(_laz1);
			terminateMachineAndNull(_laz2);
			_G(flags)[V019] = 1;
			_G(wilbur_should) = 80;

			if (!_G(flags)[V020])
				digi_preload("102w008");

			series_play_with_breaks(PLAY4, "102wi09", 256, gCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
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
			series_play_with_breaks(PLAY4, "102wi08", 256, gCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
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
	} else if (player_said("talk to")) {
		conv_load_and_prepare("conv04", 19, false);
		conv_export_value_curr(_G(flags)[V017], 0);
		conv_export_value_curr(_G(flags)[V016], 1);
		conv_play_curr();
	} else if (inv_player_has(_G(player).verb) && player_said("main street")) {
		_G(walker).wilbur_speech("102w015");
	} else if (player_said("exit") || player_said("gear", "main_street")) {
		player_set_commands_allowed(false);
		digi_preload("102_038");
		digi_play("103_038", 2, 255, 17);
	} else {
		if (player_said("TAKE", "DOORWAY"))
			_G(walker).wilbur_speech("102W003");

		if (player_said("USE", "GO", "DOORWAY")) {
			_G(wilbur_should) = 1;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}

		if (player_said("USE", "GO", "FIRE ESCAPE")) {
			_G(walker).wilbur_speech("102W005");
			kernel_trigger_dispatch_now(1003);
		}

		if (player_said("TAKE", "LAZY SUSAN")) {
			flagAction("102w007", "102h004");
		} else if (player_said("GEAR", "LAZY SUSAN")) {
			if (_G(flags)[V012]) {
				player_set_commands_allowed(false);
				_G(wilbur_should) = _G(flags)[V019] ? 82 : 79;
				kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
			} else {
				queuePlay("102h005");
			}
		} else if (player_said("GEAR", "harry") || player_said("take", "harry")) {
			_G(walker).wilbur_speech("102w015");
		} else if (player_said("take", "hair wax") || player_said("take", "hair wax ")) {
			if (_G(flags)[V012]) {
				player_set_commands_allowed(false);
				_G(wilbur_should) = player_said("hair wax") ? 71 : 75;
				kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
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
				kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
			} else {
				queuePlay("102h007");
			}
		} else if (player_said("gear", "hair wax  ") || player_said("gear", "hair wax   ")) {
			flagAction("102w012", "102h007");
		} else if (player_said("take", "back room") || player_said("gear", "back room")) {
			_G(walker).wilbur_speech("102w015");
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
			_G(walker).wilbur_speech("102w029");
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
					_G(walker).wilbur_speech("102w028");
					break;
				default:
					break;
				}
			} else {
				player_set_commands_allowed(false);
			}
		} else if (player_said("take", "towels")) {
			_G(walker).wilbur_speech("102w2029");
		} else if (player_said("gear", "towels")) {
			flagAction("102w032", "102h025");
		} else if (player_said("take", "mirror")) {
			_G(walker).wilbur_speech("102w015");
		} else if (player_said("gear", "mirror")) {
			flagAction("102w034", "102h027");
		} else if (player_said("take", "sink")) {
			_G(walker).wilbur_speech("102w037");
		} else if (player_said("gear", "sink")) {
			flagAction("102w038", "102h029");
		} else if (player_said("take", "stove")) {
			_G(walker).wilbur_speech("102w040");
		} else if (player_said("gear", "stove")) {
			// TODO
			flagAction("102w041", "102h031");
		} else if (player_said("harry") && inv_player_has(_G(player).verb) && !_G(flags)[V012]) {
			queuePlay(imath_ranged_rand(1, 2) == 1 ? "102h009y" : "102h009z");
		} else if (lookFlag) {
			if (player_said("Harry")) {
				switch (_G(kernel).trigger) {
				case -1:
					if (!_G(flags)[V012]) {
						_G(kernel).trigger_mode = KT_PARSE;
						_G(walker).wilbur_speech("102w014");
						player_set_commands_allowed(false);
					}
					break;
				case 1:
					queuePlay("102h009", 2, KT_PARSE);
					break;
				case 2:
					kernel_trigger_dispatch_now(3);
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
				_G(walker).wilbur_speech("102W020");
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
			_G(walker).wilbur_speech(name1);
		} else {
			_G(kernel).trigger_mode = KT_PARSE;
			_G(walker).wilbur_speech(name1);
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
		digi_unload(getDigi1(_G(flags)[GLB_TEMP_2]));
		digi_unload(getDigi2(_G(flags)[GLB_TEMP_2]));
	}

	if (val1) {
		_G(flags)[GLB_TEMP_2] = val1;
	} else {
		if (++_G(flags)[GLB_TEMP_2] == 36)
			_G(flags)[GLB_TEMP_2] = 1;
	}

	_val1 = 1;
	digi_preload(getDigi1(_G(flags)[GLB_TEMP_2]));
	digi_preload(getDigi2(_G(flags)[GLB_TEMP_2]));

	if (val2)
		digi_stop(3);

	digi_play_loop(getDigi2(_G(flags)[GLB_TEMP_2]), 3, 255, -1);
	_val10 = timer_read_60() + digi_ticks_to_play(getDigi2(_G(flags)[GLB_TEMP_2]));
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
		"102_011", "102_004", nullptr,   "102_006", "102_008",
		"102_009", "102_013", "102_005", "102_014", "102_023",
		"102_020", "102_011", "102_015", "102_006", "102_007",
		"102_016", "102_014", "102_015", "102_013", "102_011",
		"102_003", "102_012", "102_008", "102_013", "102_021",
		"102_011", "102_013", "102_010", "102_003", "102_005",
		"102_010", "102_011", "102_021", "102_021", "102_012"
	};

	return (num >= 1 && num <= 35) ? NAMES[num - 1] : "102_024";
}

void Room102::sub1() {
	if (_series4) {
		terminateMachineAndNull(_series4);
		terminateMachineAndNull(_series5);
	}
}

void Room102::queuePlay(const char *filename, int trigger, KernelTriggerType triggerMode) {
	if (_val9) {
		digi_stop(2);
		_val9 = 0;
		_val8 = 1;
	}

	_play1 = filename;
	_val11 = 20;
	_trigger = trigger;
	_triggerMode = triggerMode;
	player_set_commands_allowed(false);

	term_message("spleen %d %d    %d %d", _val12, _val11, 19, 20);

	if (_val12 == 20) {
		term_message("spleen2");
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatch_now(3);
	}
}

void Room102::freshen() {
	if (_G(flags)[V012] == 2) {
		player_set_commands_allowed(false);
		_G(flags)[V012] = 0;
		_val12 = 54;
		_val11 = 53;

		kernel_trigger_dispatch_now(3);
		g_vars->getInterface()->freshen_sentence();
		Section1::walk();
	}
}

void Room102::conv04() {
	error("TODO: conv04");
}

void Room102::conv05() {
	error("TODO: conv05");
}

void Room102::conv06() {
	error("TODO: conv06");
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
