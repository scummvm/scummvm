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

#include "m4/burger/rooms/section1/room103.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_CROW1_ANIMATION = 10,
	kCHANGE_CROW2_ANIMATION = 11,
	kCHANGE_CROW3_ANIMATION = 12,
	kCHANGE_CROW4_ANIMATION = 13,
	kCHANGE_HARRY_ANIMATION = 20
};

static const char *SAID[][4] = {
	{ "SATELLITE DISH", "103W001", "103W002", nullptr   },
	{ "FIRE ESCAPE",    "103W005", "103W006", nullptr   },
	{ "SCARECROW",      "103W008", "103W009", "103W009" },
	{ "DOOR",           "103W010", "103W011", nullptr   },
	{ "STAIRWAY",       "103W010", "103W011", nullptr   },
	{ "AIR VENT",       "103W013", "103W014", "103W015" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room103::SERIES1[] = {
	{   0, "103_004", 2, 255, -1,    0, nullptr, 0 },
	{   5, nullptr,   2, 255, -1, 2048, nullptr, 0 },
	{   7, "103_007", 2, 255, -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room103::SERIES2[] = {
	{ 13, "103_006", 2, 125, -1, 0, nullptr, 0 },
	{ 19, nullptr,   1, 125,  6, 0, nullptr, 0 },
	{ 54, nullptr,   1, 125, 16, 0, nullptr, 0 },
	{ -1, nullptr,   0,   0, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room103::SERIES3[] = {
	{ 5, "103_005", 2, 255, -1, 0, &_val0, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room103::SERIES4[] = {
	{ 7, "103_002", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY1[] = {
	{  1, 15, nullptr,   2, 255, -1, 0, 0, nullptr, 0 },
	{ 16, -1, "103_004", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0,  0, "103_002", 2, 100, -1, 0, 0, 0, 0 },
	{  1,  1, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  2,  2, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  1,  1, "103_002", 2, 100, -1, 0, 0, 0, 0 },
	{  0,  0, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  1,  1, "103_002", 2, 100, -1, 0, 0, 0, 0 },
	{  0,  0, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  1,  1, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  2,  2, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  1,  1, "103_002", 2, 100, -1, 0, 0, 0, 0 },
	{  0,  0, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{  1,  1, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 2, 2, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 2, 2, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 1, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{  0,  0, "103_006", 2, 255, -1, 0, 0, nullptr, 0 },
	{  0,  5, "103h006", 1, 255, 23, 0, 0, nullptr, 0 },
	{  6, 11, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

int32 Room103::_val0 = 0;

void Room103::init() {
	_flag1 = false;
	_val2 = 0;
	_val3 = 0;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		// WORKAROUND: If you save a game after moving the satellite dish, but before Harry arrives,
		// restoring the save wouldn't have Harry arrive, and you'd softlock trying to leave the roof
		_G(flags)[kHarryComingToRoof] = 0;
		break;

	case 101:
		ws_demand_location(327, 178, 7);
		_G(flags)[V012] = 0;

		player_set_commands_allowed(false);
		ws_hide_walker();
		_G(wilbur_should) = 1;
		digi_preload_stream_breaks(SERIES1);
		series_play("103wi01s", 0x100);
		series_stream_with_breaks(SERIES1, "103wi01", 6, 0xa00, 10016);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	if (_G(flags)[kHarryComingToRoof]) {
		_series1 = series_play("103wi06", 0x500, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	} else {
		_series1 = series_play("103wi05", 0x500, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	}

	_series2 = series_play("103door", 0x800, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	series_play("103vent", 0x100, 0, -1, 6, -1, 100, 0, 0, 0, 3);

	_crow1Should = 12;
	kernel_trigger_dispatch_now(kCHANGE_CROW1_ANIMATION);
	_crow3Should = 19;
	kernel_trigger_dispatch_now(kCHANGE_CROW3_ANIMATION);
	_crow4Should = 21;
	kernel_trigger_dispatch_now(kCHANGE_CROW4_ANIMATION);

	digi_play_loop("103_001", 3, 60);
}

void Room103::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		term_message("death timer");

		if (_flag1) {
			term_message("++");

			if (++_val7 < 5 || (_val2 != 0 && _val2 != 2)) {
				kernel_timing_trigger(60, 1);

			} else if (_val2 == 2) {
				kernel_timing_trigger(1, 3);
				player_set_commands_allowed(false);
				_flag1 = false;
				term_message("left");

			} else if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
				term_message("caught");
				kernel_timing_trigger(1, 8);
				intr_freshen_sentence();
				Section1::walk();
				player_set_commands_allowed(false);
				_flag1 = false;
			
			} else {
				term_message("again");
				kernel_timing_trigger(60, 1);
			}
		}
		break;

	case 2:
		if (++_val0 >= 5) {
			static const char *NAMES[7] = {
				"103h002a", "103h002b", "103h002c", "103h002d",
				"103h002e", "103h002f",  "103h002g"
			};
			assert(_digi1 >= 1 && _digi1 <= 7);
			digi_play(NAMES[_digi1 - 1], 2, 255, 1);
		} else {
			kernel_timing_trigger(60, 2);
		}
		break;

	case 3:
		terminateMachineAndNull(_series2);
		series_load("103ha03");
		series_load("103ha03s");
		digi_preload("103H005");
		digi_preload_stream_breaks(SERIES2);
		series_play("103ha01s", 0x201);
		series_stream_with_breaks(SERIES2, "103ha01", 6, 0x200, 4);
		break;

	case 4:
		digi_unload_stream_breaks(SERIES2);
		series_play_with_breaks(PLAY2, "103ha03", 0x200, 9, 3, 12);
		break;

	case 6:
		digi_play("103H005", 1, 255, 7);
		break;

	case 7:
		_G(flags)[kHarryComingToRoof] = 0;
		_G(flags)[V009] = _G(flags).get_boonsville_time_and_display() + 1800;
		_G(flags)[V012] = 2;
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1001);
		break;

	case 8:
		player_update_info();
		digi_stop(1);

		if ((_G(player_info).x > 391 && _G(player_info).y < 321) ||
				(_G(player_info).x > 490 && _G(player_info).y < 374) ||
				(_G(player_info).x > 215 && _G(player_info).y < 267) ||
				(_G(player_info).y < 224)) {
			ws_walk(271, 265, 0, 19, 2);
			term_message("walk to caught");

		} else {
			kernel_timing_trigger(1, 19);
			term_message("don't walk to caught");
		}
		break;

	case 9:
		series_play_with_breaks(PLAY3, "103ha03", 0x201, 9, 3, 12);
		series_play_with_breaks(PLAY2, "103ha03", 0x200, 9, 3, 12);
		break;

	case kCHANGE_CROW1_ANIMATION:
		switch (_crow1Should) {
		case 12:
			if (imath_ranged_rand(1, 3) == 1) {
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 30, 0, 100, 0, 0, 1, 2);
			} else {
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 60, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 13:
			switch (imath_ranged_rand(1, 8)) {
			case 1:
				_crow1Should = 12;
				Series::series_play("103cr01", 0x100, 2, kCHANGE_CROW1_ANIMATION, 30, 0, 100, 0, 0, 1, 2);
				break;

			case 3:
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 6, 0, 100, 0, 0, 7, 15);
				break;

			case 4:
			case 5:
				_crow1Should = 14;
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 30, 0, 100, 0, 0, 3, 4);
				break;

			default:
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 60, 0, 100, 0, 0, 2, 2);
				break;
			}
			break;

		case 14:
			if (imath_ranged_rand(1, 4) == 1) {
				_crow1Should = 13;
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 6, 0, 100, 0, 0, 5, 15);
			} else {
				Series::series_play("103cr01", 0x100, 0, kCHANGE_CROW1_ANIMATION, 60, 0, 100, 0, 0, 4, 4);
			}
			break;

		default:
			break;
		}
		break;

	case kCHANGE_CROW2_ANIMATION:
		switch (_crow2Should) {
		case 15:
			if (imath_ranged_rand(1, 4) == 1) {
				_crow2Should = 16;
				series_play("103cr02", 0x100, 0, kCHANGE_CROW2_ANIMATION, 30, 0, 100, 0, 0, 1, 2);
				series_play("103cr02s", 0x100, 0, -1, 30, 0, 100, 0, 0, 1, 2);
			} else {
				series_play("103cr02", 0x100, 0, kCHANGE_CROW2_ANIMATION, 70, 0, 100, 0, 0, 0, 0);
				series_play("103cr02s", 0x100, 0, -1, 70, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 16:
			switch (imath_ranged_rand(1, 5)) {
			case 1:
				_crow2Should = 15;
				series_play("103cr02", 0x100, 2, kCHANGE_CROW2_ANIMATION, 30, 0, 100, 0, 0, 1, 2);
				series_play("103cr02s", 0x100, 2, -1, 30, 0, 100, 0, 0, 1, 2);
				break;

			case 2:
				_crow2Should = 17;
				series_play("103cr02", 0x100, 0, kCHANGE_CROW2_ANIMATION, 30, 0, 100, 0, 0, 3, 4);
				series_play("103cr02s", 0x100, 0, -1, 30, 0, 100, 0, 0, 3, 4);
				break;

			default:
				series_play("103cr02", 0x100, 0, kCHANGE_CROW2_ANIMATION, 70, 0, 100, 0, 0, 2, 2);
				series_play("103cr02s", 0x100, 0, -1, 70, 0, 100, 0, 0, 2, 2);
				break;
			}
			break;

		case 17:
			switch (imath_ranged_rand(1, 6)) {
			case 1:
				_crow2Should = 16;
				series_play("103cr02", 0x100, 2, 11, 30, 0, 100, 0, 0, 3, 4);
				series_play("103cr02s", 0x100, 2, -1, 30, 0, 100, 0, 0, 3, 4);
				break;

			case 2:
				_crow2Should = 18;
				series_play("103cr02", 0x100, 0, 11, 30, 0, 100, 0, 0, 5, 8);
				series_play("103cr02s", 0x100, 0, -1, 30, 0, 100, 0, 0, 5, 8);
				break;

			default:
				series_play("103cr02", 0x100, 0, 11, 70, 0, 100, 0, 0, 4, 4);
				series_play("103cr02s", 0x100, 0, -1, 70, 0, 100, 0, 0, 4, 4);
				break;
			}
			break;

		case 18:
			if (imath_ranged_rand(1, 4) == 1) {
				_crow2Should = 17;
				series_play("103cr02", 0x100, 0, 11, 30, 0, 100, 0, 0, 9, 12);
				series_play("103cr02s", 0x100, 0, -1, 30, 0, 100, 0, 0, 9, 12);
			} else {
				series_play("103cr02", 0x100, 0, 11, 70, 0, 100, 0, 0, 8, 8);
				series_play("103cr02s", 0x100, 0, -1, 70, 0, 100, 0, 0, 8, 8);
			}
			break;

		default:
			break;
		}
		break;

	case kCHANGE_CROW3_ANIMATION:
		switch (_crow3Should) {
		case 19:
			if (imath_ranged_rand(1, 4) == 1) {
				_crow3Should = 20;
				series_play("103cr03", 0x100, 0, kCHANGE_CROW3_ANIMATION, 30, 0, 100, 0, 0, 1, 2);
				series_play("103cr03s", 0x100, 0, -1, 30, 0, 100, 0, 0, 1, 2);
			} else {
				series_play("103cr03", 0x100, 0, kCHANGE_CROW3_ANIMATION, 80, 0, 100, 0, 0, 0, 0);
				series_play("103cr03s", 0x100, 0, -1, 80, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 20:
			switch (imath_ranged_rand(1, 6)) {
			case 1:
				_crow3Should = 19;
				series_play("103cr03", 0x100, 0, kCHANGE_CROW3_ANIMATION, 80, 0, 100, 0, 0, 4, 4);
				series_play("103cr03s", 0x100, 0, -1, 80, 0, 100, 0, 0, 4, 4);
				break;

			case 3:
				series_play("103cr03", 0x100, 0, kCHANGE_CROW3_ANIMATION, 80, 0, 100, 0, 0, 2, 2);
				series_play("103cr03s", 0x100, 0, -1, 80, 0, 100, 0, 0, 2, 2);
				break;

			default:
				series_play("103cr03", 0x100, 0, kCHANGE_CROW3_ANIMATION, 80, 0, 100, 0, 0, 3, 3);
				series_play("103cr03s", 0x100, 0, -1, 80, 0, 100, 0, 0, 3, 3);
				break;
			}
			break;
		}
		break;

	case kCHANGE_CROW4_ANIMATION:
		switch (_crow4Should) {
		case 21:
			switch (imath_ranged_rand(1, 6)) {
			case 1:
				series_play("103cr04", 0x100, 0, kCHANGE_CROW4_ANIMATION, 6, 0, 100, 0, 0, 5, 9);
				series_play("103cr04s", 0x100, 0, -1, 6, 0, 100, 0, 0, 5, 9);
				break;

			case 2:
				_crow4Should = 22;
				series_play("103cr04", 0x100, 0, kCHANGE_CROW4_ANIMATION, 30, 0, 100, 0, 0, 1, 2);
				series_play("103cr04s", 0x100, 0, -1, 30, 0, 100, 0, 0, 1, 2);
				break;

			default:
				series_play("103cr04", 0x100, 0, kCHANGE_CROW4_ANIMATION, 90, 0, 100, 0, 0, 0, 0);
				series_play("103cr04s", 0x100, 0, -1, 90, 0, 100, 0, 0, 0, 0);
				break;
			}
			break;

		case 22:
			if (imath_ranged_rand(1, 4) == 1) {
				_crow4Should = 21;
				series_play("103cr04", 0x100, 2, kCHANGE_CROW4_ANIMATION, 30, 0, 100, 0, 0, 0, 1);
				series_play("103cr04s", 0x100, 2, -1, 30, 0, 100, 0, 0, 0, 1);
			} else {
				series_play("103cr04", 0x100, 0, kCHANGE_CROW4_ANIMATION, 90, 0, 100, 0, 0, 0, 2);
				series_play("103cr04s", 0x100, 0, -1, 90, 0, 100, 0, 0, 2, 2);
			}
			break;

		default:
			break;
		}
		break;

	case 14:
		_digi1 = imath_ranged_rand(1, 7);
		preloadDigi1();
		_G(flags)[kDisableFootsteps] = 1;

		if (_G(flags)[V024]) {
			player_set_commands_allowed(true);
			kernel_timing_trigger(60, 2);
		} else {
			wilbur_speech("103w003", 15);
		}

		_G(flags)[kHarryComingToRoof] = 1;
		_G(flags)[V024] = 1;
		_flag1 = true;
		_val7 = 0;
		_val0 = 0;
		break;

	case 15:
		player_set_commands_allowed(true);
		kernel_timing_trigger(1, 2);
		break;

	case 16:
		terminateMachineAndNull(_series1);
		break;

	case 17:
		_G(flags)[kHarryComingToRoof] = 0;
		_G(wilbur_should) = 6;
		_G(flags)[kDisableFootsteps] = 0;
		ws_walk(325, 173, 0, 10016);
		break;

	case 19:
		_G(flags)[kDisableFootsteps] = 1;
		terminateMachineAndNull(_series2);
		series_play_with_breaks(PLAY4, "103ha02", 0x100, kCHANGE_HARRY_ANIMATION, 2, 10, 100, 0, 0);
		_frame = 10;
		_harryShould = 9;
		break;

	case kCHANGE_HARRY_ANIMATION:
		switch (_harryShould) {
		case 9:
			if (imath_ranged_rand(1, 2) == 1) {
				if (++_frame >= 17)
					_frame = 15;
			} else {
				if (--_frame <= 10)
					_frame = 12;
			}

			series_play("103ha02", 0x101, 0, kCHANGE_HARRY_ANIMATION, 10, 0, 100, 0, 0, _frame, _frame);
			break;

		case 10:
			series_play("103ha02", 0x101, 0, 22, 6, 0, 100, 0, 0, 17, 22);
			break;

		default:
			break;
		}
		break;

	case 21:
		_harryShould = 10;
		kernel_timing_trigger(1, 24);
		break;

	case 22:
		series_play("103ha02", 0x101, 0, -1, 6, -1, 100, 0, 0, 22, 22);
		break;

	case 23:
		if (_G(flags)[V010] == 2 || _G(flags)[V013]) {
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				digi_play("103h008a", 1, 255, 21);
				break;
			case 2:
				digi_play("103h008b", 1, 255, 21);
				break;
			case 3:
				digi_play("103h008c", 1, 255, 21);
				break;
			default:
				break;
			}
		} else {
			digi_play("103h007", 1, 255, 21);
		}

		_G(flags)[V009] = _G(flags).get_boonsville_time_and_display() + 1800;
		_G(flags)[V012] = 2;
		ws_walk(271, 265, 0, 24, 2);
		break;

	case 24:
		if (++_val3 > 1) {
			wilbur_speech("103w004", 17, -1, 0, 75);
			_G(flags)[V013] = 1;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			digi_unload_stream_breaks(SERIES1);
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			break;

		case 2:
			ws_hide_walker();
			_G(wilbur_should) = 3;
			digi_preload_stream_breaks(SERIES3);
			series_play("103wi04s", 0x100, 0, -1, 6, 0, 100, 0, 0, 0, 9);
			series_stream_with_breaks(SERIES3, "103wi04", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 3:
			digi_unload_stream_breaks(SERIES3);
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("103w012");
			break;

		case 4:
			_G(wilbur_should) = 5;
			terminateMachineAndNull(_series1);
			ws_hide_walker();
			series_play("103wi03s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, 19);
			series_stream_with_breaks(SERIES4, "103wi03", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 5:
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			_series1 = series_play("103wi06", 0x500, 0, -1, 100, -1, 100, 0, 0, 0, 0);
			digi_play(Common::String::format("103h001%c",
				'a' + imath_ranged_rand(0, 4)).c_str(), 2, 255, 14);
			break;

		case 6:
			_G(wilbur_should) = 7;
			kernel_timing_trigger(1, kCHANGE_WILBUR_ANIMATION);
			break;

		case 7:
			_G(wilbur_should) = 8;
			ws_hide_walker();
			series_play_with_breaks(PLAY1, "103wi02", 0xa00, kCHANGE_WILBUR_ANIMATION, 2);
			series_play("103wi02s", 0x100, 0, -1, 6, 0, 100, 0, 0, 0, 27);
			player_set_commands_allowed(false);
			break;

		case 8:
			_val2 = 2;

			if (!_G(flags)[kHarryComingToRoof])
				pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1001);
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

void Room103::pre_parser() {
	if (player_said("gear", "satellite dish"))
		player_hotspot_walk_override(225, 257, 8);

	if (player_said("gear", "door") || player_said("gear", "stairway"))
		player_hotspot_walk_override(391, 264, 1);
}

void Room103::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(walker).wilbur_said(SAID)) {
		if (player_said("gear", "satellite dish") && _G(flags)[kHarryComingToRoof] == 0) {
			_G(wilbur_should) = 4;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			player_set_commands_allowed(false);
			_G(player).command_ready = false;

		} else if (player_said("gear", "door") || player_said("gear", "stairway")) {
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			player_set_commands_allowed(false);
			_G(player).command_ready = false;

		} else if (player_said("ENTER", "FIRE ESCAPE") || player_said("gear", "FIRE ESCAPE")) {
			_val2 = 1;
			_G(wilbur_should) = 6;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			player_set_commands_allowed(false);
			_G(player).command_ready = false;
		}
	} else {
		_G(player).command_ready = false;
	}
}

void Room103::preloadDigi1() {
	const char *NAMES[7] = {
		"103h002a", "103h002b", "103h002c", "103h002d",
		"103h002e", "103h002f", "103h002g"
	};

	assert(_digi1 >= 1 && _digi1 <= 7);
	digi_preload(NAMES[_digi1 - 1]);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
