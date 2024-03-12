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

#include "m4/burger/rooms/section9/room971.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room971::SERIES1[] = {
	{   0, "971intro", 1, 255,   -1, 0, nullptr, 0 },
	{  79, "971ship",  2, 255,   -1, 0, nullptr, 0 },
	{ 109, nullptr,    0,   0,   56, 0, nullptr, 0 },
	{ 110, "972birds", 3, 100, 1024, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES2[] = {
	{   0, "971d001",  1, 255, -1, 0, nullptr, 0 },
	{   0, "971tunes", 3, 100, -1, 0, nullptr, 0 },
	{  77, "971run",   2, 160, -1, 0, nullptr, 0 },
	{ 122, "971open",  2, 200, -1, 0, nullptr, 0 },
	{ 125, "971slam",  2, 200, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES3[] = {
	{   0, "972birds", 3,  80, -1, 1024, 0, 0 },
	{   1, "972d001",  1, 255, -1,    0, 0, 0 },
	{  35, "971slam",  2,  80, -1,    0, 0, 0 },
	{  53, "972run_1", 2, 100, -1,    0, 0, 0 },
	{ 110, "972dream", 2, 150, -1,    0, 0, 0 },
	{ 129, nullptr,    0,   0, 56,    0, 0, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES4[] = {
	{  0, "973ambi",  3,  80, -1, 1024, nullptr, 0 },
	{  1, "973d001",  1, 255, -1,    0, nullptr, 0 },
	{  1, nullptr,    0,   0, 57,    0, nullptr, 0 },
	{  1, "973zap_1", 2, 155, -1,    0, nullptr, 0 },
	{ 14, "973zap_2", 2, 155, -1,    0, nullptr, 0 },
	{ 25, "973ducto", 2,  80, -1,    0, nullptr, 0 },
	{ 64, "973whirl", 2, 155, -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES5[] = {
	{  1, "975d001",  1, 255, -1, 0, nullptr, 0 },
	{ 36, "975zlick", 2, 155, -1, 0, nullptr, 0 },
	{ 48, "975zeat",  2, 155, -1, 0, nullptr, 0 },
	{ 56, "975zeat",  2, 155, -1, 0, nullptr, 0 },
	{ 64, "975zgulp", 2, 155, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES6[] = {
	{  3, "976d001",    1, 255, -1, 0, nullptr, 0 },
	{  9, "976spark",   2, 155, -1, 0, nullptr, 0 },
	{ 15, "976spark",   2, 155, -1, 0, nullptr, 0 },
	{ 29, "976clamp",   2, 100, -1, 0, nullptr, 0 },
	{ 43, "976tv",      3, 100, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES7[] = {
	{  0, "977ambi",  3,  50, -1, 1024, nullptr, 0 },
	{  1, "977d001",  1, 255, -1,    0, nullptr, 0 },
	{ 32, "977wahh",  2, 120, -1,    0, nullptr, 0 },
	{ 43, "977zlaff", 2, 120, -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES8[] = {
	{ 1, "978d001",  1, 255, -1,    0, nullptr, 0 },
	{ 11, "978lick", 2, 200, -1,    0, nullptr, 0 }, 
	{ 19, "978eat",  2, 200, -1,    0, nullptr, 0 }, 
	{ 31, "978gulp", 2, 200, -1,    0, nullptr, 0 }, 
	{ 54, "978lick", 2, 200, -1,    0, nullptr, 0 }, 
	{ 64, "978eat",  2, 200, -1,    0, nullptr, 0 }, 
	{ 76, "978gulp", 2, 200, -1,    0, nullptr, 0 }, 
	{ 82, nullptr,   3,   0, -1, 2048, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES9[] = {
	{ 0, "972birds", 3,  70, -1, 1024, nullptr, 0 },
	{ 1, "972d002",  1, 255, -1,    0, nullptr, 0 },
	{ 3, "972run_2", 2,  80, -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES10[] = {
	{  0, "980ambi",  3, 150, -1, 1024, nullptr, 0 },
	{ 10, "980d001",  1, 255, -1,    0, nullptr, 0 },
	{ 46, "980smash", 2, 255, -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES11[] = {
	{  0, "981ambi", 3, 70,  -1, 0, nullptr, 0 },
	{  2, "981cut",  2, 150, -1, 0, nullptr, 0 },
	{ 12, "981cut",  2, 150, -1, 0, nullptr, 0 },
	{ 26, "981cut",  2, 150, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES12[] = {
	{  1, "982p001", 1, 255, -1, 0, nullptr, 0 },
	{ 23, "982gun",  2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES13[] = {
	{  0, "984ambi", 3, 60, -1, 1024, 0, 0 },
	{ 10, "984d001", 1, 255, -1,   0, 0, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES14[] = {
	{ 36, "985w001", 2, 225, -1,    0, nullptr, 0 },
	{ 48, "985a001", 1, 225, -1,    0, nullptr, 0 },
	{ 68, nullptr,   3,   0, -1, 2048, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES15[] = {
	{   0, "972birds", 3, 70, -1, 1024, nullptr, 0 },
	{   0, "972d003",  1, 255, -1,   0, nullptr, 0 }, 
	{  84, "972d004",  1, 255, -1,   0, nullptr, 0 }, 
	{ 149, "972w001",  2, 255, -1,   0, nullptr, 0 }, 
	{ 219, "972ducto", 1, 155, -1,   0, nullptr, 0 }, 
	{ 232, "972w002",  2, 255, -1,   0, nullptr, 0 }, 
	{ 255, "972d005",  1, 255, -1,   0, nullptr, 0 }, 
	{ 325, "972glug",  2, 155, -1,   0, nullptr, 0 }, 
	{ 335, nullptr,    0,   0, 46,   0, nullptr, 0 },
	{ 338, nullptr,    0,   0, 56,   0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room971::SERIES16[] = {
	STREAM_BREAK_END
};

void Room971::preload() {
	palette_prep_for_stream();
	_G(player).walker_in_this_scene = false;
}

void Room971::init() {
	palette_prep_for_stream();
	_G(kernel).suppress_fadeup = true;
	pal_fade_set_start(0);
	kernel_timing_trigger(1, 1);
}

void Room971::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		palette_prep_for_stream();
		digi_stop(1);
		digi_stop(2);
		digi_stop(3);
		digi_unload("971outro");
		digi_unload_stream_breaks(SERIES16);
		digi_preload_stream_breaks(SERIES1);
		digi_preload_stream_breaks(SERIES2);
		digi_preload_stream_breaks(SERIES3);
		digi_preload_stream_breaks(SERIES4);
		digi_preload_stream_breaks(SERIES5);

		series_stream_with_breaks(SERIES1, "971title", 6, 1, 2);
		pal_fade_init(0, 255, 100, 60, -1);
		break;

	case 2:
		palette_prep_for_stream();
		release_trigger_on_digi_state(3, 3);
		break;

	case 3:
		digi_unload("971intro");
		digi_unload("971ship");
		palette_prep_for_stream();
		series_stream_with_breaks(SERIES2, "971a", 6, 1, 4);
		pal_fade_init(0, 255, 100, 60, -1);
		break;

	case 4:
		palette_prep_for_stream();
		kernel_timing_trigger(5, 6);
		break;

	case 5:
		release_trigger_on_digi_state(6, 1);
		break;

	case 6:
		series_stream_with_breaks(SERIES3, "972a", 6, 1, 7);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 7:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 8);
		break;

	case 8:
		compact_mem_and_report();
		release_trigger_on_digi_state(9, 1);
		break;

	case 9:
		series_stream_with_breaks(SERIES4, "973a", 6, 1, 10);
		pal_fade_init(0, 255, 100, 120, -1);
		break;

	case 10:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 11);
		break;

	case 11:
		compact_mem_and_report();
		release_trigger_on_digi_state(12, 1);
		break;

	case 12:
		series_stream_with_breaks(SERIES5, "975a", 6, 1, 13);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 13:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 14);
		break;

	case 14:
		digi_unload_stream_breaks(SERIES2);
		digi_unload_stream_breaks(SERIES3);
		digi_unload_stream_breaks(SERIES4);
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES6);
		release_trigger_on_digi_state(15, 1);
		break;

	case 15:
		series_stream_with_breaks(SERIES6, "976a", 6, 1, 16);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 16:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 17);
		break;

	case 17:
		digi_unload_stream_breaks(SERIES5);
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES7);
		release_trigger_on_digi_state(18, 1);
		break;

	case 18:
		series_stream_with_breaks(SERIES7, "977a", 6, 1, 19);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 19:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 20);
		break;

	case 20:
		digi_unload_stream_breaks(SERIES6);
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES8);
		release_trigger_on_digi_state(21, 1);
		break;

	case 21:
		series_stream_with_breaks(SERIES8, "978a", 6, 1, 22);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 22:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 23);
		break;

	case 23:
		digi_unload_stream_breaks(SERIES7);
		compact_mem_and_report();
		release_trigger_on_digi_state(24, 1);
		break;

	case 24:
		series_stream_with_breaks(SERIES9, "972b", 6, 1, 25);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 25:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 26);
		break;

	case 26:
		digi_unload_stream_breaks(SERIES8);
		digi_unload_stream_breaks(SERIES9);
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES10);
		digi_preload_stream_breaks(SERIES11);
		digi_preload_stream_breaks(SERIES12);
		release_trigger_on_digi_state(30, 1);
		break;

	case 30:
		series_stream_with_breaks(SERIES10, "980a", 6, 1, 31);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 31:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 32);
		break;

	case 32:
		compact_mem_and_report();
		kernel_trigger_dispatch_now(33);
		break;

	case 33:
		series_stream_with_breaks(SERIES11, "981a", 6, 1, 34);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 34:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 35);
		break;

	case 35:
		compact_mem_and_report();
		release_trigger_on_digi_state(36, 1);
		break;

	case 36:
		series_stream_with_breaks(SERIES12, "982a", 6, 1, 37);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 37:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 38);
		break;

	case 38:
		digi_unload_stream_breaks(SERIES10);
		digi_unload_stream_breaks(SERIES11);
		digi_unload_stream_breaks(SERIES12);
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES13);
		digi_preload_stream_breaks(SERIES14);
		release_trigger_on_digi_state(39, 1);
		break;

	case 39:
		series_stream_with_breaks(SERIES13, "984a", 6, 1, 40);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 40:
		palette_prep_for_stream();
		kernel_trigger_dispatch_now(42);
		break;

	case 42:
		// The Ultimate answer to Life, The Universe, and Everything
		series_stream_with_breaks(SERIES14, "985a", 6, 1, 43);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 43:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 44);
		break;

	case 44:
		digi_unload_stream_breaks(SERIES13);
		digi_unload_stream_breaks(SERIES14);
		compact_mem_and_report();
		digi_preload_stream_breaks(SERIES15);
		digi_preload("971outro");
		release_trigger_on_digi_state(45, 1);
		break;

	case 45:
		series_stream_with_breaks(SERIES15, "972c", 6, 1, 47);
		pal_fade_init(0, 255, 100, 15, -1);
		break;

	case 46:
		digi_play("971outro", 3, 200, 53);
		break;

	case 47:
		palette_prep_for_stream();
		kernel_timing_trigger(6, 48);
		break;

	case 48:
		digi_unload_stream_breaks(SERIES15);
		compact_mem_and_report();
		release_trigger_on_digi_state(49, 1);
		break;

	case 49:
		palette_prep_for_stream();
		_comeSoonS = series_load("comesoon", -1, _G(master_palette));
		_comeSoon = series_show("comesoon", 0);
		pal_fade_init(0, 255, 100, 120, 50);
		break;

	case 50:
		kernel_timing_trigger(480, 51);
		digi_preload_stream_breaks(SERIES1);
		digi_preload_stream_breaks(SERIES2);
		digi_preload_stream_breaks(SERIES3);
		digi_preload_stream_breaks(SERIES4);
		digi_preload_stream_breaks(SERIES5);
		break;

	case 51:
		pal_fade_init(0, 255, 0, 120, 52);
		break;

	case 52:
		terminateMachineAndNull(_comeSoon);
		series_unload(_comeSoonS);
		digi_preload_stream_breaks(SERIES16);
		series_stream_with_breaks(SERIES16, "swmclogo", 6, 1, -1);
		pal_fade_init(0, 255, 100, 60, -1);
		break;

	case 53:
		pal_fade_init(0, 255, 0, 120, 58);
		break;

	case 54:
		pal_fade_init(0, 255, 0, 30, -1);
		break;

	case 55:
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 56:
		pal_fade_init(0, 255, 0, 60, -1);
		break;

	case 57:
		pal_fade_init(0, 255, 100, 60, -1);
		break;

	case 58:
		switch (_G(executing)) {
		case JUST_OVERVIEW:
			kernel_trigger_dispatch_now(1);
			break;

		case INTERACTIVE_DEMO:
			_G(game).new_room = 901;
			player_set_commands_allowed(false);
			break;

		case WHOLE_GAME:
			_G(game).new_room = 903;
			player_set_commands_allowed(false);
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

} // namespace Rooms
} // namespace Burger
} // namespace M4
