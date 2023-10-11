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

#include "m4/burger/rooms/section8/room802.h"
#include "m4/burger/rooms/section8/section8.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak SERIES1[] = {
	{   0, "802_012", 1, 150, -1, 0, nullptr, 0 },
	{   8, "802_003", 2, 175, -1, 0, nullptr, 0 },
	{  24, "802_010", 2, 255, -1, 0, nullptr, 0 },
	{  33, "802f004", 1, 255, -1, 0, nullptr, 0 },
	{  46, "801_003", 2, 150, -1, 0, nullptr, 0 },
	{  63, nullptr,   0,   0, 24, 0, nullptr, 0 },
	{  66, "802_007", 2, 150, -1, 0, nullptr, 0 },
	{  68, "802f005", 1, 255, -1, 0, nullptr, 0 },
	{ 112, "802f006", 1, 255, -1, 0, nullptr, 0 },
	{ 194, "802f007", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{  0, "806B_001", 3, 100, -1, 0, nullptr, 0 },
	{  6, "806z001",  1, 255, -1, 0, nullptr, 0 },
	{ 83, "806f001",  1, 255, -1, 0, nullptr, 0 },
	{ 98, "806A_002", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{   4, "803_005", 2,  75, -1, 0, nullptr, 0 },
	{   5, "803f002", 1, 255, -1, 0, nullptr, 0 },
	{  15, "803_006", 2,  75, -1, 0, nullptr, 0 },
	{  19, "803_007", 2,  75, -1, 0, nullptr, 0 },
	{  30, "803_005", 2,  75, -1, 0, nullptr, 0 },
	{  34, "803_006", 2,  75, -1, 0, nullptr, 0 },
	{  40, "803_007", 2,  75, -1, 0, nullptr, 0 },
	{  40, "803z004", 1, 255, -1, 0, nullptr, 0 },
	{  57, "803_005", 2,  75, -1, 0, nullptr, 0 },
	{  61, "803_006", 2,  75, -1, 0, nullptr, 0 },
	{  78, "803_007", 2,  75, -1, 0, nullptr, 0 },
	{  82, "803_005", 2,  75, -1, 0, nullptr, 0 },
	{  99, "803_006", 2,  75, -1, 0, nullptr, 0 },
	{ 103, "803_007", 2,  75, -1, 0, nullptr, 0 },
	{ 115, "803z005", 1, 255, -1, 0, nullptr, 0 },
	{ 131, "803f003", 1, 255, -1, 0, nullptr, 0 },
	{ 133, "803_005", 2,  75, -1, 0, nullptr, 0 },
	{ 137, "803_006", 2,  75, -1, 0, nullptr, 0 },
	{ 147, "803f004", 1, 255, -1, 0, nullptr, 0 },
	{ 154, "803_007", 2,  75, -1, 0, nullptr, 0 },
	{ 158, "803_005", 2,  75, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES4[] = {
	{  6, "802f003", 1, 255, -1, 0, nullptr, 0 },
	{  8, "802w002", 2, 255, -1, 0, nullptr, 0 },
	{ 22, "802_018", 1, 150, -1, 0, nullptr, 0 },
	{ 26, "802_005", 2,  75, -1, 0, nullptr, 0 },
	{ 40, "802_006", 2,  75, -1, 0, nullptr, 0 },
	{ 76, "802_008", 2,  75, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY1[] = {
	{ 2, 5, "802_008", 2, 100, -1, 1, 2, nullptr, 0 },
	{ 5, 5, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 6, 9, "802_013", 2, 100, -1, 1,  1, nullptr, 0 },
	{ 0, 0, nullptr,   0,   0, -1, 0, 10, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 0, 4, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 4, 4, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 4, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY5[] = {
	{  0, 7, 0, 0, 0, -1, 0, 0, 0, 0 },
	{  7, 7, 0, 0, 0, -1, 0, 0, 0, 0 },
	{  7, 0, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY6[] = {
	{  0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0  },
	{  8, 14, "802_004", 2, 255, -1, 0, 0, nullptr, 0  },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0  },
	PLAY_BREAK_END
};


void Room802::init() {
	_G(player).walker_in_this_scene = false;
	interface_hide();
	player_set_commands_allowed(false);
	digi_preload("800_001");
	pal_fade_set_start(0);
	pal_fade_init(0, 255, 100, 40, -1);

	_val1 = 11;
	_val2 = 2;
	_flag1 = false;
	_flag2 = false;

	if (_G(flags)[kNEURO_TEST_COUNTER] > 1) {
		kernel_trigger_dispatch_now(24);
		digi_preload_stream_breaks(SERIES1);
		kernel_trigger_dispatch_now(14);
	} else {
		kernel_trigger_dispatch_now(1);
	}
}

void Room802::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_preload_stream_breaks(SERIES2);
		digi_preload_stream_breaks(SERIES3);
		digi_preload_stream_breaks(SERIES4);
		loadSeries();
		digi_preload_stream_breaks(SERIES1);

		digi_stop(1);
		digi_stop(2);
		digi_stop(3);
		digi_unload("800w001");
		digi_unload("807_002");
		digi_unload("807_001");

		series_stream_with_breaks(SERIES2, "806A", 6, 1, 2);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 2:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 3);
		break;

	case 3:
		switch (_G(flags)[kNEURO_TEST_COUNTER]) {
		case 0:
			release_trigger_on_digi_state(4, 1);
			break;
		case 1:
			kernel_trigger_dispatch_now(24);
			release_trigger_on_digi_state(7, 1);
			break;
		default:
			kernel_trigger_dispatch_now(24);
			release_trigger_on_digi_state(14, 1);
			break;
		}
		break;

	case 4:
		kernel_trigger_dispatch_now(24);
		series_stream_with_breaks(SERIES3, "803B", 6, 1, 5);
		pal_fade_init(0, 255, 100, 30, -1);
		kernel_timing_trigger(1, 23);
		break;

	case 5:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 6);
		break;

	case 6:
		pal_cycle_stop();
		compact_mem_and_report();
		release_trigger_on_digi_state(7, 1);
		break;

	case 7:
		digi_unload_stream_breaks(SERIES2);
		series_stream_with_breaks(SERIES4, "802B1", 6, 1, 10);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 8:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 9);
		break;

	case 9:
		compact_mem_and_report();
		release_trigger_on_digi_state(10, 1);
		break;

	case 10:
		digi_preload("802_019");
		digi_play_loop("802_019", 3, 128);

		if (_G(flags)[kNEURO_TEST_COUNTER] == 0) {
			player_set_commands_allowed(true);
			digi_unload_stream_breaks(SERIES3);

			_G(wilbur_should) = getWilburShould();
			_G(flumix_should) = 6;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			kernel_trigger_dispatch_now(17);

			_series1 = series_play("802BFX03", 0x101, 0, -1, 6, -1);
			_series2 = series_play("802BFX02", 0x100, 1, -1, 6, -1);
			conv_load_and_prepare("conv81", 13);
			conv_play_curr();

		} else {
			digi_unload_stream_breaks(SERIES3);
			kernel_trigger_dispatch_now(14);
		}
		break;

	case 13:
		_G(wilbur_should) = 19;
		_G(flumix_should) = 19;

		if (_flag2 && _flag1) {
			terminateMachineAndNull(_series5);
			_flag2 = false;
			_series3.terminate();
			_flag1 = false;
			terminateMachineAndNull(_series1);
			terminateMachineAndNull(_series2);
			kernel_trigger_dispatch_now(14);

		} else {
			kernel_timing_trigger(30, 13);
		}
		break;

	case 14:
		digi_stop(1);
		digi_stop(2);
		digi_stop(3);
		digi_unload("806w001");
		digi_unload("807_002");
		digi_unload("807_001");
		digi_preload("802_019");
		digi_play_loop("802_019", 3, 128);
		digi_unload_stream_breaks(SERIES4);
		digi_unload_play_breaks(PLAY1);
		digi_unload_play_breaks(PLAY2);
		series_stream_with_breaks(SERIES1, "802B3", 6, 1, 15);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 15:
		disable_player_commands_and_fade_init(16);
		break;

	case 16:
		release_trigger_on_digi_state(k10027, 1);
		break;

	case 17:
		// Flumix
		switch (_val2) {
		case 1:
			switch (_G(flumix_should)) {
			case 5:
				_flag1 = true;
				_val2 = 3;
				_series3.play("802BF03", 0x100, 0, 17, 6);
				break;

			case 6:
				_flag1 = true;
				_val2 = 2;
				_series3.play("802BF03", 0x100, 0, 17, 6);
				break;

			case 9:
				_flag1 = false;
				_G(flumix_should) = getFlumixShould();
				series_play_with_breaks(PLAY2, "802BF02", 0x100, 17, 3);
				break;

			case 10:
				_flag1 = false;
				_G(flumix_should) = getFlumixShould();
				_val4 = 30;
				series_play_with_breaks(PLAY3, "802BF02", 0x100, 17, 1);
				break;

			case 19:
				_flag1 = true;
				_series3.show("802BF02", 0x100, 0, -1, -1, 5);
				break;

			default:
				_flag1 = false;
				_val4 = 30;
				_G(flumix_should) = getFlumixShould();
				series_play_with_breaks(PLAY1, "802BF02", 0x100, 17, 3);
				break;
			}
			break;

		case 2:
			switch (_G(flumix_should)) {
			case 5:
				_flag1 = true;
				_val2 = 3;
				_series3.show("802BF01", 0x100, 0, 17, 1);
				break;

			case 6 :
				_flag1 = true;
				_G(flumix_should) = getFlumixShould();
				_series3.show("802BF03", 0x100, 0, 17, 30, 1);
				break;

			case 19:
				_flag1 = true;
				_series3.show("802BF02", 0x100, 0, -1, -1, 5);
				break;

			default:
				_flag1 = true;
				_val2 = 1;
				_series3.show("802BF03", 0x100, 2, 17, 6);
				break;
			}
			break;

		case 3:
			switch (_G(flumix_should)) {
			case 5:
				_flag1 = true;
				_G(flumix_should) = 20;
				_G(wilbur_should) = 16;
				_series3.play("802BF01", 0x100, 4, -1, 6, -1);
				digi_play(conv_sound_to_play(), 1, 255, 17);
				break;

			case 19:
				_flag1 = true;
				_series3.show("802BF02", 0x100, 0, -1, -1, 5);
				break;

			case 20:
				_series3.terminate();
				_G(flumix_should) = getFlumixShould();
				_flag1 = true;
				_series3.show("802BF01", 0x100, 0, 17, 30, 0);
				conv_resume_curr();
				break;

			default:
				_val2 = 1;
				_flag1 = true;
				_series3.play("802BF03", 0x100, 2, 17, 6);
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 19:
		if (conv_sound_to_play()) {
			int who = conv_whos_talking();
			if (who <= 0)
				_G(flumix_should) = 5;
			else if (who == 1)
				_G(wilbur_should) = 14;
		}
		break;

	case 22:
		pal_fade_init(0, 255, 0, 30, -1);
		break;

	case 23:
		pal_mirror_colours(119, 122);
		gr_pal_set_range(119, 8);
		pal_cycle_init(119, 126, 6, -1, -1);
		break;

	case 24:
		digi_play_loop("800_001", 3, 48, -1, 800);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_val1) {
		case 11:
			switch (_G(wilbur_should)) {
			case 14:
				_flag2 = true;
				_G(wilbur_should) = 20;
				_G(flumix_should) = 6;
				_series5 = series_play("802BW03", 0x100, 4, -1, 6, -1);
				digi_play(conv_sound_to_play(), 1, 255, kCHANGE_WILBUR_ANIMATION, 802);
				break;

			case 15:
			case 18:
				_flag2 = true;
				_val1 = 13;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				break;

			case 19:
				if (_flag2)
					terminateMachineAndNull(_series5);

				_flag2 = true;
				_series5 = series_show("802BW01", 0x100, 0, -1, -1, 0);
				break;

			case 20:
				terminateMachineAndNull(_series5);
				_flag2 = true;
				_G(wilbur_should) = 16;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				conv_resume_curr();
				break;

			default:
				_flag2 = true;
				_val1 = 12;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				break;
			}
			break;

		case 12:
			switch (_G(wilbur_should)) {
			case 14:
				_flag2 = true;
				_G(flumix_should) = 6;
				_val1 = 11;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				break;

			case 15:
			case 18:
				_flag2 = true;
				_val1 = 13;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				break;

			case 16:
				_flag2 = true;
				_G(wilbur_should) = getWilburShould();
				_series5 = series_show("802BW01", 0x100, 0, kCHANGE_WILBUR_ANIMATION, 30, 1);
				break;

			case 19:
				if (_flag2)
					terminateMachineAndNull(_series5);

				_flag2 = true;
				_series5 = series_show("802BW01", 0x100, 0, -1, -1, 0);
				break;

			default:
				_flag2 = false;
				_val5 = imath_ranged_rand(15, 30);
				_G(wilbur_should) = getWilburShould();
				series_play_with_breaks(PLAY4, "802BW01", 0x100, kCHANGE_WILBUR_ANIMATION, 0, 12);
				break;
			}
			break;

		case 13:
			switch (_G(wilbur_should)) {
			case 14:
			case 16:
			case 17:
				_flag2 = true;
				_val1 = 12;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				break;

			case 18:
				_flag2 = false;
				_G(wilbur_should) = 16;
				_val1 = 12;
				_val5 = imath_ranged_rand(120, 180);
				series_play_with_breaks(PLAY5, "802BW02", 0x100, kCHANGE_WILBUR_ANIMATION);
				break;

			case 19:
				if (_flag2)
					terminateMachineAndNull(_series5);

				_flag2 = true;
				_series5 = series_show("802BW01", 0x100, 0, -1, -1, 0);
				break;

			default:
				_flag2 = false;
				_G(wilbur_should) = 16;
				_val1 = 12;
				series_play_with_breaks(PLAY6, "802BW04", 0x100, kCHANGE_WILBUR_ANIMATION);
				break;
			}
			break;

		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room802::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv81")) {
		kernel_trigger_dispatch_now(19);
		_G(player).command_ready = false;
	}
}

void Room802::loadSeries() {
	digi_preload_play_breaks(PLAY1);
	digi_preload_play_breaks(PLAY2);
	digi_preload_play_breaks(PLAY6);

	static const char *NAMES[12] = {
		"802BFX02", "802BFX03", "802BF01", "802BF01S", "802BF02", "802BF02S",
		"802BF03", "802BF03S", "802BW01", "802BW02", "802BW03", "802BW04"
	};
	for (int i = 0; i < 12; ++i)
		series_load(NAMES[i]);
}

int Room802::getWilburShould() {
	switch (imath_ranged_rand(1, 16)) {
	case 1:
	case 2:
	case 3:
		return 17;
	case 11:
		return 18;
	case 16:
		return 15;
	default:
		return 16;
	}
}

int Room802::getFlumixShould() {
	switch (imath_ranged_rand(1, 16)) {
	case 1:
	case 3:
		return 8;
	case 5:
	case 7:
		return 9;
	case 9:
	case 11:
	case 13:
		return 6;
	default:
		return 10;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
