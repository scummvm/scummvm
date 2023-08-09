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

static const seriesPlayBreak PLAY1[] = {
	{ 0, 7, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8, 8, nullptr, 1, 255,  2, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0, 18, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room102::init() {
	_G(player).walker_in_this_scene = true;
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
		ws_demand_location(_G(my_walker), 321, 343);
		ws_demand_facing(_G(my_walker), 4);
		ws_hide_walker(_G(my_walker));

	} else {
		_G(roomVal1) = 1;
		_val16 = 1;
		digi_preload("102_038");
		digi_play("102_038", 2, 255, gTELEPORT);
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
				series_play("102ha01", 0, 3, 4, 0, 100, 0, 0, 24, 24);
				break;

			case 21:
				_val12 = 21;
				series_play("102ha01", 0, 3, 4, 0, 100, 0, 0, 30, 30);
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
				TerminateMachineAndNull(_series2);
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
					TerminateMachineAndNull(_series3);

				_series3 = series_play("102ha01", 2560, 4, -1, 5, -1, 100, 0, 0, 24, 29);
			} else {
				TerminateMachineAndNull(_series3);
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
				TerminateMachineAndNull(_series3);
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
				TerminateMachineAndNull(_series1);
				TerminateMachineAndNull(_series2);
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
				TerminateMachineAndNull(_series3);
				TerminateMachineAndNull(_series6);
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
				TerminateMachineAndNull(_series3);
				TerminateMachineAndNull(_series6);
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

	// TODO: More daemon stuff

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room102::pre_parser() {
}

void Room102::parser() {
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
		TerminateMachineAndNull(_series4);
		TerminateMachineAndNull(_series5);
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

} // namespace Rooms
} // namespace Burger
} // namespace M4
