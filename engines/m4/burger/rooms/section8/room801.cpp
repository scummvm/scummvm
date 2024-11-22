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

#include "m4/burger/rooms/section8/room801.h"
#include "m4/burger/rooms/section8/section8.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_FLUMIX_ANIMATION = 15,
	kCHANGE_ZLARG_ANIMATION = 16
};

static const seriesStreamBreak SERIES1[] = {
	{ 0, "805_001", 3,  80, -1, 0, nullptr, 0 },
	{ 1, "805w001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{  1, "806f002", 1, 255, -1, 0, nullptr, 0 },
	{ 22, "806z002", 1, 255, -1, 0, nullptr, 0 },
	{ 36, "806f003", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{   0, "803w001", 1, 255, -1, 0, nullptr, 0 },
	{  59, "803z006", 1, 255, -1, 0, nullptr, 0 },
	{  86, "803z007", 1, 255, -1, 0, nullptr, 0 },
	{ 116, "803z008", 1, 255, -1, 0, nullptr, 0 },
	{ 182, "803f005", 1, 255, -1, 0, nullptr, 0 },
	{ 220, "803z009", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES4[] = {
	{  5, "802_014", 2, 255, -1, 0, nullptr, 0 },
	{ 16, "802f008", 1, 255, -1, 0, nullptr, 0 },
	{ 28, "802f009", 1, 255, -1, 0, nullptr, 0 },
	{ 87, "802_015", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES5[] = {
	{  0, "807_001", 3, 255, -1, 0, nullptr, 0 },
	{  0, "807_002", 2, 255, -1, 0, nullptr, 0 },
	{  1, "804_003", 2, 255, -1, 0, nullptr, 0 },
	{  3, "807_002", 2, 255, -1, 0, nullptr, 0 },
	{  5, nullptr,   0,   0, 42, 0, nullptr, 0 },
	{  6, "804_003", 2, 255, -1, 0, nullptr, 0 },
	{  9, "807_002", 2, 255, -1, 0, nullptr, 0 },
	{ 12, "804_003", 2, 255, -1, 0, nullptr, 0 },
	{ 15, "807_002", 2, 255, -1, 0, nullptr, 0 },
	{ 28, "807_003", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES6[] = {
	{  0, nullptr,   0,   0, 37, 0, nullptr, 0 },
	{  5, "801w001", 1, 255, -1, 0, nullptr, 0 },
	{ 10, "801_001", 2,  75, -1, 0, nullptr, 0 },
	{ 33, "801_001", 2,  75, -1, 0, nullptr, 0 },
	{ 56, "801_001", 2,  75, -1, 0, nullptr, 0 },
	{ 74, "801_003", 2, 200, -1, 0, nullptr, 0 },
	{ 75, "801w002", 1, 255, -1, 0, nullptr, 0 },
	{ 97, nullptr,   0,   0, 38, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES7[] = {
	{   0, nullptr,   0,   0, 37, 0, nullptr, 0 },
	{   7, "801_003", 1, 175, -1, 0, nullptr, 0 },
	{  23, "802_017", 2,  75, -1, 0, nullptr, 0 },
	{  54, nullptr,   0,   0, 41, 0, nullptr, 0 },
	{  61, "802f001", 1, 255, -1, 0, nullptr, 0 },
	{ 115, "802w002", 2, 255, -1, 0, nullptr, 0 },
	{ 119, "802f002", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES8[] = {
	{   0, "803_008", 2, 175, -1, 0, nullptr, 0 },
	{  10, "803_009", 1, 255, -1, 0, nullptr, 0 },
	{  10, "803_002", 2, 200, -1, 0, nullptr, 0 },
	{  32, "803z001", 1, 255, -1, 0, nullptr, 0 },
	{  52, "803f001", 1, 255, -1, 0, nullptr, 0 },
	{  58, "803_003", 2,  75, -1, 0, nullptr, 0 },
	{  68, "803z002", 1, 255, -1, 0, nullptr, 0 },
	{ 160, "803z003", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES9[] = {
	{   3, "804_002", 2, 150, -1, 0, nullptr, 0 },
	{  14, "804_003", 2, 150, -1, 0, nullptr, 0 },
	{  17, "804_003", 2, 150, -1, 0, nullptr, 0 },
	{  23, "804_004", 2, 150, -1, 0, nullptr, 0 },
	{  39, "804z001", 1, 255, -1, 0, nullptr, 0 },
	{  72, "804f002", 1, 255, -1, 0, nullptr, 0 },
	{  93, "804z002", 1, 255, -1, 0, nullptr, 0 },
	{ 109, "804z003", 1, 255, -1, 0, nullptr, 0 },
	{ 177, "804f003", 1, 255, -1, 0, nullptr, 0 },
	{ 204, "804f004", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

void Room801::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room801::init() {
	const char *HEADER = "+++++++++++++++++++++++++++++++++++++++++++++++++++++++";
	interface_hide();
	player_set_commands_allowed(false);
	pal_fade_set_start(0);
	pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
	_wilburMode = 10;
	digi_preload("800_001");

	if (_G(flags)[V011] == 1) {
		term_message(HEADER);
		term_message("Toxic wax in hair");

		digi_preload_stream_breaks(SERIES1);
		digi_preload_stream_breaks(SERIES2);
		digi_preload_stream_breaks(SERIES3);
		digi_preload_stream_breaks(SERIES4);
		digi_preload_stream_breaks(SERIES5);

		kernel_trigger_dispatch_now(20);
	} else {
		switch (_G(flags)[kNEURO_TEST_COUNTER]) {
		case 0:
			term_message(HEADER);
			term_message("FIRST ATTEMPT AT NEURO TEST");

			digi_preload_stream_breaks(SERIES6);
			digi_preload_stream_breaks(SERIES7);
			digi_preload_stream_breaks(SERIES8);

			pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
			kernel_trigger_dispatch_now(1);
			break;

		case 1:
			term_message(HEADER);
			term_message("SECOND ATTEMPT AT NEURO TEST");
			digi_preload_stream_breaks(SERIES8);
			kernel_trigger_dispatch_now(4);
			break;

		case 2:
			term_message(HEADER);
			term_message("THIRD ATTEMPT AT NEURO TEST");
			digi_preload_stream_breaks(SERIES8);
			kernel_trigger_dispatch_now(41);
			kernel_trigger_dispatch_now(7);
			break;

		default:
			term_message(HEADER);
			term_message("FOURTH OR GREATER ATTEMPT AT NEURO TEST");
			digi_preload_stream_breaks(SERIES5);
			kernel_trigger_dispatch_now(20);
			break;
		}
	}
}

void Room801::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		series_stream_with_breaks(SERIES6, "801A", 6, 1, 2);
		kernel_timing_trigger(1, 39);
		break;

	case 2:
		kernel_timing_trigger(6, 3);
		break;

	case 3:
		compact_mem_and_report();
		release_trigger_on_digi_state(4, 1, 0);
		break;

	case 4:
		digi_play_loop("802_002", 3);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		series_stream_with_breaks(SERIES7, "802A", 6, 1, 5);
		kernel_timing_trigger(1, 39);
		break;

	case 5:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 6);
		break;

	case 6:
		compact_mem_and_report();

		if (_G(flags)[kNEURO_TEST_COUNTER] == 0) {
			release_trigger_on_digi_state(7, 3);
		} else {
			release_trigger_on_digi_state(10, 3);
		}
		break;

	case 7:
		digi_unload_stream_breaks(SERIES6);
		series_stream_with_breaks(SERIES8, "803A", 6, 1, 8);
		pal_fade_init(0, 255, 100, 30, -1);
		kernel_timing_trigger(1, 40);
		break;

	case 8:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 9);
		break;

	case 9:
		pal_cycle_stop();
		compact_mem_and_report();

		if (_G(flags)[kNEURO_TEST_COUNTER] == 2) {
			release_trigger_on_digi_state(20, 1);
		} else {
			release_trigger_on_digi_state(10, 1);
		}
		break;

	case 10:
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES9);
		_series1 = series_load("804FX01");
		_series2 = series_load("804FX02");
		_series3 = series_load("804FX03");
		_series4 = series_load("804FX04");
		series_stream_with_breaks(SERIES9, "804A", 6, 1, 11);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 11:
		digi_unload_stream_breaks(SERIES8);
		compact_mem_and_report();

		if (_G(flags)[kNEURO_TEST_COUNTER] == 0) {
			kernel_trigger_dispatch_now(13);
		} else {
			_G(wilbur_should) = getWilburShould();
			_G(flumix_should) = 2;
			_zlargShould = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			kernel_trigger_dispatch_now(kCHANGE_ZLARG_ANIMATION);
			kernel_trigger_dispatch_now(kCHANGE_FLUMIX_ANIMATION);

			loadSeries1();
			kernel_timing_trigger(6, 18);
		}
		break;

	case 12:
		compact_mem_and_report();
		release_trigger_on_digi_state(13, 1);
		break;

	case 13:
		kernel_trigger_dispatch_now(41);
		loadSeries1();
		loadSeries2();

		_G(wilbur_should) = getWilburShould();
		_G(flumix_should) = 2;
		_zlargShould = 2;

		_series9 = series_play("804FL01", 2, 0, -1, 6, -1);
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(kCHANGE_ZLARG_ANIMATION);
		kernel_trigger_dispatch_now(kCHANGE_FLUMIX_ANIMATION);
		conv_load_and_prepare("conv80", 18);
		player_set_commands_allowed(true);
		conv_play_curr();
		break;

	case 14:
		// Conversation handler
		if (conv_sound_to_play()) {
			int who = conv_whos_talking();
			int node = conv_current_node();
			int entry = conv_current_entry();

			if (who <= 0) {
				if ((node == 3 && entry == 0) || (node == 4 && entry == 0) ||
					(node == 6 && entry == 1) || (node == 8 && entry == 0)) {
					_zlargShould = 13;
				} else {
					_G(flumix_should) = 13;
				}
			} else if (who == 1) {
				_G(wilbur_should) = 5;
			}
		}
		break;

	case kCHANGE_FLUMIX_ANIMATION:
		switch (_G(flumix_should)) {
		case 3:
			terminateMachineAndNull(_series10);
			_series10 = series_show("804FLX01", 1, 0, -1, -1, 8);
			digi_play("804_006", 2);
			kernel_timing_trigger(60, 19);
			break;

		case 12:
			terminateMachineAndNull(_series10);
			_G(flumix_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_FLUMIX_ANIMATION);
			conv_resume_curr();
			break;

		case 13:
			_G(flumix_should) = 12;
			_G(wilbur_should) = 6;
			_series10 = series_play("804FLT01", 1, 4, -1, 6, -1);
			digi_play(conv_sound_to_play(), 1, 255, kCHANGE_FLUMIX_ANIMATION);
			break;

		default:
			_G(flumix_should) = 2;
			_series10 = series_show("804FL01", 1, 0, kCHANGE_FLUMIX_ANIMATION, 30, 0);
			break;
		}
		break;

	case kCHANGE_ZLARG_ANIMATION:
		switch (_zlargShould) {
		case 2:
			if (imath_ranged_rand(0, 15) <= 12) {
				_zlargShould = 2;
				_series11 = series_show("804ZL01", 1, 0, kCHANGE_ZLARG_ANIMATION, 30, 0);
			} else {
				_zlargShould = 1;
				kernel_trigger_dispatch_now(kCHANGE_ZLARG_ANIMATION);
			}
			break;

		case 12:
			terminateMachineAndNull(_series11);
			_zlargShould = 2;
			kernel_trigger_dispatch_now(kCHANGE_ZLARG_ANIMATION);
			conv_resume_curr();
			break;

		case 13:
			_zlargShould = 12;
			_G(wilbur_should) = 4;
			_series11 = series_play("804ZLT01", 1, 4, -1, 6, -1);
			digi_play(conv_sound_to_play(), 1, 255, kCHANGE_ZLARG_ANIMATION);
			break;

		default:
			_zlargShould = 2;
			_series11 = series_play("804ZFX01", 1, 0, kCHANGE_ZLARG_ANIMATION, 6, 0);
			break;
		}
		break;

	case 18:
		// Beginning the first test
		terminateMachineAndNull(_series10);
		terminateMachineAndNull(_series9);
		_G(flumix_should) = 3;
		_series10 = series_play("804FLX01", 1, 16, -1, 6, 0, 100, 0, 0, 0, 7);
		kernel_timing_trigger(180, kCHANGE_FLUMIX_ANIMATION);
		break;

	case 19:
		pal_fade_set_start(0);

		terminateMachineAndNull(_series12);
		terminateMachineAndNull(_series10);
		terminateMachineAndNull(_series11);

		freeSeries1();
		digi_unload_stream_breaks(SERIES7);
		digi_unload("804_005");
		kernel_trigger_dispatch_now(20);
		break;

	case 20:
		player_set_commands_allowed(false);
		digi_preload_stream_breaks(SERIES5);
		digi_preload("806w001");
		adv_kill_digi_between_rooms(false);
		series_stream_with_breaks(SERIES5, "807A", 6, 1, 21);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 21:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 22);
		digi_unload_stream_breaks(SERIES9);
		break;

	case 22:
		compact_mem_and_report();
		kernel_trigger_dispatch_now(23);
		break;

	case 23:
		if (_G(flags)[V011] == 1) {
			kernel_trigger_dispatch_now(24);
		} else if (_G(flags)[kNEURO_TEST_COUNTER] <= 2) {
			digi_unload("804_003");
			digi_unload("807_002");
			adv_kill_digi_between_rooms(false);
			digi_play_loop("806w001", 1, 128, -1, 806);
			digi_play_loop("807_003", 2, 255, -1, 807);
			digi_play_loop("807_001", 3, 255, -1, 807);

			_G(game).new_room = 802;
		} else {
			kernel_trigger_dispatch_now(k10027);
		}
		break;

	case 24:
		series_stream_with_breaks(SERIES1, "805A", 6, 1, 25);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 25:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 26);
		break;

	case 26:
		compact_mem_and_report();
		release_trigger_on_digi_state(27, 1);
		break;

	case 27:
		digi_unload_stream_breaks(SERIES5);
		kernel_trigger_dispatch_now(41);
		series_stream_with_breaks(SERIES2, "806B", 6, 1, 28);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 28:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 29);
		break;

	case 29:
		compact_mem_and_report();
		release_trigger_on_digi_state(30, 1);
		break;

	case 30:
		digi_unload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES3, "803c", 6, 1, 31);
		pal_fade_init(0, 255, 100, 30, -1);
		kernel_timing_trigger(1, 40);
		break;

	case 31:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 32);
		break;

	case 32:
		compact_mem_and_report();
		release_trigger_on_digi_state(33, 1);
		break;

	case 33:
		pal_cycle_stop();
		digi_unload_stream_breaks(SERIES2);
		series_stream_with_breaks(SERIES4, "802C", 6, 1, 34);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 34:
		pal_fade_set_start(0);
		digi_unload_stream_breaks(SERIES3);
		kernel_timing_trigger(6, 35);
		break;

	case 35:
		compact_mem_and_report();
		release_trigger_on_digi_state(36, 1);
		break;

	case 36:
		_G(flags)[kFirstTestPassed] = 1;
		adv_kill_digi_between_rooms(1);
		kernel_trigger_dispatch_now(k10027);
		break;

	case 37:
		pal_fade_init(_G(kernel).first_fade, 255, 100, 30, -1);
		break;

	case 38:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, -1);
		break;

	case 39:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		break;
		
	case 40:
		pal_mirror_colours(119, 122, _G(master_palette));
		gr_pal_set_range(119, 8);
		pal_cycle_init(119, 126, 6, -1, -1);
		break;

	case 41:
		digi_play_loop("800_001", 3, 48, -1, 800);
		break;

	case 42:
		if (_G(flags)[V011] != 1)
			digi_play("806w001", 1, 128);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_wilburMode) {
		case 9:
			switch (_G(wilbur_should)) {
			case 5:
			case 6:
			case 7:
				_series12 = series_play("804WI04", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 6, 0);
				_wilburMode = 11;
				break;

			default:
				_G(wilbur_should) = getWilburShould();
				_series12 = series_show("804Wi04", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 30, 1);
				break;
			}
			break;

		case 10:
			switch (_G(wilbur_should)) {
			case 4:
			case 7:
				_series12 = series_play("804WI05", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 6, 0);
				_wilburMode = 11;
				break;

			case 5:
				_G(wilbur_should) = 12;
				_series12 = series_play("804WI01", 0x200, 4, -1, 6, -1);
				digi_play(conv_sound_to_play(), 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 12:
				terminateMachineAndNull(_series12);
				_G(wilbur_should) = getWilburShould();
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				conv_resume_curr();
				break;

			default:
				_G(wilbur_should) = getWilburShould();
				_series12 = series_show("804WI05", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 30, 3);
				break;
			}
			break;

		case 11:
			switch (_G(wilbur_should)) {
			case 4:
				_series12 = series_play("804WI04", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 6, 0);
				_wilburMode = 9;
				break;

			case 5:
			case 6:
				_series12 = series_play("804WI05", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 6, 0);
				_wilburMode = 10;
				break;

			case 7:
				_G(wilbur_should) = getWilburShould();
				_series12 = series_play("804WI03", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 6, 0);
				break;

			default:
				_G(wilbur_should) = getWilburShould();
				_series12 = series_play("804WI02", 0x200, 0, kCHANGE_WILBUR_ANIMATION);
				break;
			}
			break;

		default:
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room801::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv80")) {
		kernel_trigger_dispatch_now(14);
		_G(player).command_ready = false;
	}
}

int Room801::getWilburShould() const {
	switch (imath_ranged_rand(0, 15)) {
	case 1:
	case 2:
		return 7;
	case 6:
	case 7:
		return 6;
	case 12:
		return 4;
	default:
		return 8;
	}
}

void Room801::loadSeries1() {
	_series5 = series_play("804FX01", 0x400, 0, -1, 10, -1);
	_series6 = series_play("804FX02", 0x400, 0, -1, 45, -1);
	_series7 = series_play("804FX03", 0x400, 0, -1, 10, -1);
	_series8 = series_play("804FX04", 0x400, 0, -1, 10, -1);
}

void Room801::loadSeries2() {
	static const char *NAMES[16] = {
		"804FX01", "804FX02", "804FX03", "804FX04", "804Wi01", "804Wi02",
		"804Wi03", "804Wi04", "804Wi05", "804FL01", "804ZL01", "804FL01",
		"804FLT01", "804ZLT01", "804FLX01", "804ZFX01"
	};

	digi_preload("804_005");
	for (int i = 0; i < 16; ++i)
		series_load(NAMES[i]);
}

void Room801::freeSeries1() {
	terminateMachineAndNull(_series5);
	terminateMachineAndNull(_series6);
	terminateMachineAndNull(_series7);
	terminateMachineAndNull(_series8);
	series_unload(_series1);
	series_unload(_series2);
	series_unload(_series3);
	series_unload(_series4);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
