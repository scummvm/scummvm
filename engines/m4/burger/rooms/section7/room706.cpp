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

#include "m4/burger/rooms/section7/room706.h"
#include "m4/burger/rooms/section7/section7.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_ASTRAL_ANIMATION = 33,
	kCHANGE_FLUMIX_ANIMATION = 34
};

static const seriesStreamBreak SERIES1[] = {
	{   0, "706F001",  1, 255, -1, 0, nullptr, 0 },
	{  31, "706W001",  1, 255, -1, 0, nullptr, 0 }, 
	{  52, "706F002",  1, 255, -1, 0, nullptr, 0 }, 
	{  83, "706A_001", 2, 150, -1, 0, nullptr, 0 }, 
	{ 105, "706A_002", 2, 125, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{   0, "706w002",  1, 255, -1, 0, nullptr, 0 },
	{  50, "706F003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 103, "706W003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 140, "706F004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 145, "706A_003", 2, 200, -1, 0, nullptr, 0 }, 
	{ 170, "706W004",  1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{   3, "707A_002", 1, 125, -1, 0, nullptr, 0 },
	{  11, "707A_001", 2, 175, -1, 0, nullptr, 0 }, 
	{  32, "707W001",  1, 255, -1, 0, nullptr, 0 }, 
	{  56, "707F001",  1, 255, -1, 0, nullptr, 0 }, 
	{ 100, "707W002",  1, 255, -1, 0, nullptr, 0 }, 
	{ 104, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 115, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 120, "707F002",  1, 255, -1, 0, nullptr, 0 }, 
	{ 134, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 147, "707W003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 163, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 180, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 190, "707F003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 192, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 201, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 214, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 222, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 233, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 237, "707W004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 239, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 253, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 259, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 267, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 268, "707F004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 271, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 277, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 281, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 295, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 303, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 317, "707F005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 323, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 345, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 347, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 365, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 379, "707Z001",  1, 255, -1, 0, nullptr, 0 }, 
	{ 389, "707Z002",  1, 255, -1, 0, nullptr, 0 }, 
	{ 450, "707Z003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 473, "707F006",  1, 255, -1, 0, nullptr, 0 }, 
	{ 478, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 491, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 509, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 523, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 529, "707A_007", 2, 255, -1, 0, nullptr, 0 }, 
	{ 535, "707Z005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 536, "707A_007", 2, 255, -1, 0, nullptr, 0 }, 
	{ 543, "707Z004a", 1, 255, -1, 0, nullptr, 0 }, 
	{ 544, "707A_007", 2, 255, -1, 0, nullptr, 0 }, 
	{ 550, "707Z005a", 1, 255, -1, 0, nullptr, 0 }, 
	{ 556, "707Z007",  1, 255, -1, 0, nullptr, 0 }, 
	{ 589, "707F007",  1, 255, -1, 0, nullptr, 0 }, 
	{ 618, "707Z008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 645, "707F008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 686, "707F009",  1, 255, -1, 0, nullptr, 0 }, 
	{ 710, "707W005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 711, "707A_006", 2, 100, -1, 0, nullptr, 0 }, 
	{ 718, "707A_006", 2, 100, -1, 0, nullptr, 0 }, 
	{ 725, "707A_006", 2, 100, -1, 0, nullptr, 0 }, 
	{ 729, "707Z009",  1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES4[] = {
	{   0, "708Z001", 1, 255, -1, 0, nullptr, 0 },
	{  48, "708W001", 1, 255, -1, 0, nullptr, 0 },  
	{ 107, "708Z002", 1, 255, -1, 0, nullptr, 0 }, 
	{ 187, "708W002", 1, 255, -1, 0, nullptr, 0 }, 
	{ 206, "708Z003", 1, 255, -1, 0, nullptr, 0 }, 
	{ 296, "708A001", 1, 200, -1, 0, nullptr, 0 }, 
	{ 312, "708Z004", 1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES5[] = {
	{ 0, "707A001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES6[] = {
	{ 0, "708Z005", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES7[] = {
	{   0, "707C_001", 3, 100, -1, (uint)-1, nullptr, 0 },
	{   1, nullptr,    0,   0, 37,  0, nullptr, 0 },
	{   2, "707A002",  1, 255, -1,  0, nullptr, 0 }, 
	{  33, "707Z010",  1, 255, -1,  0, nullptr, 0 },  
	{  95, "707C_002", 2, 200, -1,  0, nullptr, 0 },  
	{ 100, "707Z011",  1, 255, -1,  0, nullptr, 0 }, 
	{ 124, "707F010",  1, 255, -1,  0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES8[] = {
	{  0, "709A_004", 3,  75, -1, (uint)-1, nullptr, 0 },
	{  0, "709W001",  1, 255, -1,  0, nullptr, 0 }, 
	{  5, "709A_002", 2, 110, -1,  0, nullptr, 0 }, 
	{ 16, "709A001",  1, 255, -1,  0, nullptr, 0 },
	{ 41, "709A_003", 2,  75, -1,  0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES9[] = {
	{   0, "707D_005", 3, 75, -1, (uint)-1, nullptr, 0 },
	{   0, "707W006",  1, 255, -1, 0, nullptr, 0 },
	{  16, "707W007",  1, 255, -1, 0, nullptr, 0 },
	{  28, nullptr,    0,   0, 35, 0, nullptr, 0 },
	{  29, "707D_001", 2, 150, -1, 0, nullptr, 0 },
	{  31, "707A003",  1, 255, -1, 0, nullptr, 0 },
	{  41, "707A004",  1, 255, -1, 0, nullptr, 0 }, 
	{  84, "707W008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 126, "707W009",  1, 255, -1, 0, nullptr, 0 }, 
	{ 132, nullptr,    0,   0, 36, 0, nullptr, 0 },
	{ 134, "707D_006", 3, 100, -1, 0, nullptr, 0 },
	{ 138, "707A_003", 2,  50, -1, 0, nullptr, 0 },  
	{ 141, "707A_003", 2,  50, -1, 0, nullptr, 0 },  
	{ 143, "707A_002", 2, 125, -1, 0, nullptr, 0 }, 
	{ 158, "707A005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 175, "707W010",  1, 255, -1, 0, nullptr, 0 }, 
	{ 223, "707W010z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 320, "707A006",  1, 255, -1, 0, nullptr, 0 }, 
	{ 353, "707W011",  1, 125, -1, 0, nullptr, 0 }, 
	{ 364, "707W012",  1, 255, -1, 0, nullptr, 0 }, 
	{ 453, "707W013",  1, 125, -1, 0, nullptr, 0 }, 
	{ 475, "707A007",  1, 255, -1, 0, nullptr, 0 }, 
	{ 600, "707W014",  1, 255, -1, 0, nullptr, 0 }, 
	{ 601, nullptr,    0,   0, 38, 0, nullptr, 0 },
	{ 650, "707A008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 676, "707W015",  1, 255, -1, 0, nullptr, 0 }, 
	{ 696, "707W016",  1, 255, -1, 0, nullptr, 0 }, 
	{ 698, "707D_008", 2, 200, -1, 0, nullptr, 0 }, 
	{ 715, "707D_004", 2, 150, -1, 0, nullptr, 0 }, 
	{ 729, "707A009",  1, 255, -1, 0, nullptr, 0 }, 
	{ 730, "707D_008", 2, 200, -1, 0, nullptr, 0 }, 
	{ 753, "707W017",  1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES10[] = {
	{  0, "709A002",   1, 255, -1,    0, nullptr, 0 },
	{  45, "709B_001", 2, 100, -1,    0, nullptr, 0 }, 
	{  56, "709A003",  1, 255, -1,    0, nullptr, 0 }, 
	{  83, "709W002",  2, 255, -1,    0, nullptr, 0 }, 
	{  92, "709A004",  1, 255, -1,    0, nullptr, 0 }, 
	{  95, "709B_002", 2, 255, -1,    0, nullptr, 0 }, 
	{ 103, "709W003",  1, 255, -1,    0, nullptr, 0 }, 
	{ 110, nullptr,    0,   0, 39,    0, nullptr, 0 },
	{ 115, nullptr,    0,   0, 40,    0, nullptr, 0 },
	{ 120, "709B_006", 2, 255, -1,    0, nullptr, 0 }, 
	{ 124, "709B_004", 3,  50, -1, 1024, nullptr, 0 }, 
	{ 264, nullptr,    0,   0, 41,    0, nullptr, 0 },
	STREAM_BREAK_END
};

void Room706::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room706::init() {
	player_set_commands_allowed(false);
	pal_fade_set_start(0);
	pal_fade_init(0, 255, 100, 40, -1);
	_flag5 = false;
	_flag6 = false;
	_flag7 = false;
	_volume = 100;

	_series1 = series_load("706FLX01");
	digi_preload("706A_004");
	digi_preload_stream_breaks(SERIES1);
	_series2 = series_load("706AST01");
	_series3 = series_load("706AST02");
	_series4 = series_load("706WIT01");

	digi_preload_stream_breaks(SERIES2);
	kernel_trigger_dispatch_now(1);
}

void Room706::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_play_loop("706A_004", 3, 50, -1, 706);
		series_stream_with_breaks(SERIES1, "706A1", 6, 1, 2);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 2:
		kernel_timing_trigger(6, 3);
		break;

	case 3:
		compact_mem_and_report();
		kernel_timing_trigger(6, 4);
		break;

	case 4:
		player_set_commands_allowed(true);
		getFlumixShould();
		kernel_trigger_dispatch_now(kCHANGE_FLUMIX_ANIMATION);
		conv_load_and_prepare("conv83", 5);
		conv_play_curr();
		break;

	case 5:
		if (_flag5)
			terminateMachineAndNull(_series5);
		if (_flag7) {
			terminateMachineAndNull(_series7);
			terminateMachineAndNull(_series8);
		}
		if (_flag6)
			terminateMachineAndNull(_series6);

		_flag5 = _flag6 = _flag7 = false;
		_G(flumix_should) = 4;
		series_unload(_series2);
		series_unload(_series3);
		series_unload(_series4);
		kernel_timing_trigger(6, 6);
		break;

	case 6:
		digi_unload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES2, "706A3", 6, 1, 7);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 7:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 8);
		break;

	case 8:
		compact_mem_and_report();
		kernel_timing_trigger(6, 9);
		break;

	case 9:
		series_unload(_series1);
		digi_unload_stream_breaks(SERIES2);
		digi_unload("706A_004");
		digi_preload("700_001");
		digi_preload_stream_breaks(SERIES3);
		digi_play_loop("700_001", 3, 50, -1, 700);
		series_stream_with_breaks(SERIES3, "707A", 6, 1, 10);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 10:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 11);
		break;

	case 11:
		compact_mem_and_report();
		kernel_timing_trigger(6, 12);
		break;

	case 12:
		digi_unload_stream_breaks(SERIES3);
		digi_preload_stream_breaks(SERIES4);
		digi_preload("707B_001");
		digi_preload_stream_breaks(SERIES5);
		digi_preload_stream_breaks(SERIES6);
		digi_preload_stream_breaks(SERIES7);
		series_stream_with_breaks(SERIES4, "708A", 6, 1, 13);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 13:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 14);
		break;

	case 14:
		compact_mem_and_report();
		kernel_timing_trigger(6, 15);
		break;

	case 15:
		digi_unload_stream_breaks(SERIES4);
		digi_play("707B_001", 2, 125);
		series_stream_with_breaks(SERIES5, "707B", 6, 1, 16);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 16:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 17);
		break;

	case 17:
		compact_mem_and_report();
		kernel_timing_trigger(6, 18);
		break;

	case 18:
		series_stream_with_breaks(SERIES6, "708B", 6, 1, 19);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 19:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 20);
		break;

	case 20:
		compact_mem_and_report();
		kernel_timing_trigger(6, 21);
		break;

	case 21:
		digi_unload_stream_breaks(SERIES5);
		digi_unload_stream_breaks(SERIES6);
		series_stream_with_breaks(SERIES7, "707C", 6, 1, 22);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 22:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 23);
		break;

	case 23:
		compact_mem_and_report();
		kernel_timing_trigger(6, 24);
		break;

	case 24:
		digi_unload_stream_breaks(SERIES7);
		digi_preload_stream_breaks(SERIES8);
		series_stream_with_breaks(SERIES8, "709A", 6, 1, 25);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 25:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 26);
		break;

	case 26:
		compact_mem_and_report();
		kernel_timing_trigger(6, 27);
		break;

	case 27:
		digi_unload_stream_breaks(SERIES8);
		digi_preload_stream_breaks(SERIES9);
		digi_preload("707D_007");
		series_stream_with_breaks(SERIES9, "707D", 6, 1, 28);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 28:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 29);
		break;

	case 29:
		compact_mem_and_report();
		kernel_timing_trigger(6, 30);
		break;

	case 30:
		digi_unload_stream_breaks(SERIES9);
		digi_preload("709B_005");
		digi_preload_stream_breaks(SERIES10);
		series_stream_with_breaks(SERIES10, "709B", 6, 1, 31);
		pal_fade_init(0, 255, 100, 31, -1);
		break;

	case 31:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 32);
		digi_unload_stream_breaks(SERIES10);
		break;

	case 32:
		compact_mem_and_report();
		kernel_timing_trigger(6, 42);
		break;

	case kCHANGE_ASTRAL_ANIMATION:
		switch (_astralShould) {
		case 1:
			_astralShould = 21;
			_flag7 = true;
			_series7 = series_play("706AST01", 0x300, 0, -1, 6, -1);
			_series8 = series_play("706AST02", 0x300, 0, -1, 6, -1);
			digi_play(conv_sound_to_play(), 1, 255, kCHANGE_ASTRAL_ANIMATION);
			break;

		case 21:
			if (_flag7) {
				terminateMachineAndNull(_series7);
				terminateMachineAndNull(_series8);
			}

			_flag7 = false;
			conv_resume_curr();
			break;

		default:
			break;
		}
		break;

	case kCHANGE_FLUMIX_ANIMATION:
		switch (_G(flumix_should)) {
		case 2:
			getFlumixShould();
			series_play("706FLX01", 0x400, 0, kCHANGE_FLUMIX_ANIMATION, 6);
			break;

		case 3:
			getFlumixShould();
			kernel_timing_trigger(60, kCHANGE_FLUMIX_ANIMATION);
			break;

		default:
			_G(flumix_should) = 4;
			break;
		}
		break;

	case 35:
		inv_move_object("MIRROR", NOWHERE);
		break;

	case 36:
		digi_stop(3);
		break;

	case 37:
		digi_stop(2);
		digi_unload("707B_001");
		break;

	case 38:
		digi_play_loop("707D_007", 3, 100, -1, 707);
		break;

	case 39:
		_volume -= 20;
		if (_volume > 0) {
			term_message("fading hero tune, current volume = %d", _volume);
			digi_change_volume(3, _volume);
			kernel_timing_trigger(20, 39);
	
		} else {
			digi_stop(3);
		}
		break;

	case 40:
		digi_play("709B_005", 1, 255, -1, 709);
		break;

	case 41:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 150, -1);
		break;

	case 42:
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(_G(executing) == WHOLE_GAME ? 903 : 901);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			_G(wilbur_should) = 21;
			_flag6 = true;
			_series6 = series_play("706WIT01", 0x200, 0, -1, 6, -1);
			digi_play(conv_sound_to_play(), 1, 255, kCHANGE_WILBUR_ANIMATION);
			break;

		case 21:
			if (_flag6)
				terminateMachineAndNull(_series6);

			_flag6 = false;
			conv_resume_curr();
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

void Room706::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv83")) {
		conv83();
		_G(player).command_ready = false;
	}
}

void Room706::conv83() {
	if (conv_sound_to_play()) {
		int who = conv_whos_talking();

		if (who <= 0) {
			_astralShould = 1;
			kernel_trigger_dispatch_now(kCHANGE_ASTRAL_ANIMATION);

		} else if (who == 1) {
			_G(wilbur_should) = 1;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
	}
}

void Room706::getFlumixShould() {
	_G(flumix_should) = imath_ranged_rand(1, 100) >= 10 ? 3 : 2;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
