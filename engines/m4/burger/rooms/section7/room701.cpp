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

#include "m4/burger/rooms/section7/room701.h"
#include "m4/burger/rooms/section7/section7.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak SERIES1[] = {
	{    0, "700Z001",  1, 255, -1, 0, 0, 0 },
	{   64, "700A_001", 2, 200, -1, 0, 0, 0 },
	{   66, "700W001",  1, 255, -1, 0, 0, 0 },
	{   68, "700F001",  2, 255, -1, 0, 0, 0 },
	{   92, "700A_002", 2, 255, -1, 0, 0, 0 },
	{   96, "700Z002",  1, 255, -1, 0, 0, 0 },
	{  130, "700F002",  1, 255, -1, 0, 0, 0 },
	{  172, "700W002",  1, 255, -1, 0, 0, 0 },
	{  208, "700F003",  1, 255, -1, 0, 0, 0 },
	{  268, "700Z003",  1, 255, -1, 0, 0, 0 },
	{  350, "700F004",  1, 255, -1, 0, 0, 0 },
	{  368, "700F005",  1, 255, -1, 0, 0, 0 },
	{  415, "700Z004",  1, 255, -1, 0, 0, 0 },
	{  476, "700F006",  1, 255, -1, 0, 0, 0 },
	{  527, "700F007",  1, 255, -1, 0, 0, 0 },
	{  565, "700W003",  1, 255, -1, 0, 0, 0 },
	{  601, "700F008",  1, 255, -1, 0, 0, 0 },
	{  769, "700Z005",  1, 255, -1, 0, 0, 0 },
	{  796, "700Z006",  1, 255, -1, 0, 0, 0 },
	{  869, "700Z007",  1, 255, -1, 0, 0, 0 },
	{  910, "700F009",  1, 255, -1, 0, 0, 0 },
	{  945, "700Z008",  1, 255, -1, 0, 0, 0 },
	{  989, "700Z009",  1, 255, -1, 0, 0, 0 },
	{ 1028, "700C001",  1, 255, -1, 0, 0, 0 },
	{ 1073, "700F010",  1, 255, -1, 0, 0, 0 },
	{ 1085, "700F011",  1, 255, -1, 0, 0, 0 },
	{ 1095, "700F012",  1, 255, -1, 0, 0, 0 },
	{ 1105, "700F013",  1, 255, -1, 0, 0, 0 },
	{ 1119, "700F014",  1, 255, -1, 0, 0, 0 },
	{ 1145, "700F015",  1, 255, -1, 0, 0, 0 },
	{ 1175, "700F016",  1, 255, -1, 0, 0, 0 },
	{ 1189, "700Z010",  1, 255, -1, 0, 0, 0 },
	{ 1227, "700Z011",  1, 255, -1, 0, 0, 0 },
	{ 1255, "700C002",  1, 255, -1, 0, 0, 0 },
	{ 1296, "700A001",  1, 255, -1, 0, 0, 0 },
	{ 1317, "700Z012",  1, 255, -1, 0, 0, 0 }, 
	{ 1318, "700A_003", 2, 255, -1, 0, 0, 0 }, 
	{ 1335, "700A_004", 2, 255, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{  0, "701_002", 2, 255, -1, 0, nullptr, 0 },
	{  2, "701Z001", 1, 255, -1, 0, nullptr, 0 },
	{ 15, "700A002", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{   8, "700Z013",  1, 255, -1, 0, nullptr, 0 },
	{  12, "700B_001", 2, 255, -1, 0, nullptr, 0 },
	{  64, "700A003",  1, 255, -1, 0, nullptr, 0 },
	{  74, nullptr,    0,   0, 25, 0, nullptr, 0 },
	{ 121, "700Z014",  1, 255, -1, 0, nullptr, 0 }, 
	{ 137, "700Z015",  1, 255, -1, 0, nullptr, 0 }, 
	{ 181, "700W004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 226, "700Z016",  1, 255, -1, 0, nullptr, 0 }, 
	{ 312, "700A005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 331, "700Z017",  1, 255, -1, 0, nullptr, 0 }, 
	{ 356, "700Z018",  1, 255, -1, 0, nullptr, 0 }, 
	{ 399, "700Z018z", 1, 255, -1, 0, nullptr, 0 },
	{ 415, "700B_002", 2, 255, -1, 0, nullptr, 0 }, 
	{ 439, "700B_003", 2, 255, -1, 0, nullptr, 0 }, 
	{ 472, "700Z019",  1, 200, -1, 0, nullptr, 0 }, 
	{ 510, "700A006",  1, 255, -1, 0, nullptr, 0 }, 
	{ 533, "700Z020",  1, 255, -1, 0, nullptr, 0 }, 
	{ 641, "700Z020z", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES4[] = {
	{  6, "702W016", 1, 255, -1, 0, nullptr, 0 },
	{ 24, "702_001", 2,  75, -1, 0, nullptr, 0 },
	{ 30, "702_002", 2, 100, -1, 0, nullptr, 0 },
	{ 35, "702W001", 1, 255, -1, 0, nullptr, 0 },
	{ 55, nullptr,   0,   0, 22, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES5[] = {
	{  0, "703W001", 1, 255, -1, 0, nullptr, 0 },
	{ 88, "703W002", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES6[] = {
	{   7, "702F001",  1, 255, -1, 0, nullptr, 0 },	
	{  27, "702F001z", 1, 255, -1, 0, nullptr, 0 },
	{  67, "702F002",  1, 255, -1, 0, nullptr, 0 },
	{  95, "702_003",  2, 175, -1, 0, nullptr, 0 },
	{ 120, "702_004",  2, 255, -1, 0, nullptr, 0 },
	{ 126, "702f004",  1, 255, -1, 0, nullptr, 0 },
	{ 128, "702_005",  2, 255, -1, 0, nullptr, 0 },
	{ 130, nullptr,    0,   0, 22, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES7[] = {
	{  0, "704F001", 1, 255, -1, 0, nullptr, 0 },
	{ 16, nullptr,   1,   0, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room701::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room701::init() {
	player_set_commands_allowed(false);
	_G(flags).set_boonsville_time(505210);
	pal_fade_set_start(0);
	pal_fade_init(0, 255, 100, 40, -1);
	digi_preload("700_001");
	digi_preload_stream_breaks(SERIES1);
	digi_preload_stream_breaks(SERIES2);
	kernel_trigger_dispatch_now(1);
}

void Room701::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_play_loop("700_001", 3, 50, -1, 700);
		series_stream_with_breaks(SERIES1, "700A", 6, 1, 2);
		pal_fade_init(0, 255, 100, 30, -1);
		kernel_timing_trigger(1, 23);
		break;

	case 2:
		pal_fade_set_start(0);
		pal_cycle_stop();
		kernel_timing_trigger(6, 3);
		break;

	case 3:
		compact_mem_and_report();
		kernel_timing_trigger(6, 4);
		break;

	case 4:
		series_stream_with_breaks(SERIES2, "701A", 6, 1, 5);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 5:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 6);
		break;

	case 6:
		compact_mem_and_report();
		kernel_timing_trigger(6, 7);
		break;

	case 7:
		digi_unload_stream_breaks(SERIES1);
		digi_unload_stream_breaks(SERIES3);
		series_stream_with_breaks(SERIES3, "700B", 6, 1, 8);
		pal_fade_init(0, 255, 100, 30, -1);
		kernel_timing_trigger(1, 24);
		break;

	case 8:
		pal_fade_set_start(0);
		pal_cycle_stop();
		kernel_timing_trigger(6, 9);
		break;

	case 9:
		compact_mem_and_report();
		kernel_timing_trigger(6, 10);
		break;

	case 10:
		digi_unload_stream_breaks(SERIES2);
		digi_unload_stream_breaks(SERIES4);
		series_stream_with_breaks(SERIES4, "702A", 6, 1, 11);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 11:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 12);
		break;

	case 12:
		compact_mem_and_report();
		kernel_timing_trigger(6, 13);
		break;

	case 13:
		digi_unload_stream_breaks(SERIES3);
		digi_unload_stream_breaks(SERIES5);
		digi_preload_stream_breaks(SERIES6);
		series_stream_with_breaks(SERIES5, "703A", 6, 1, 14);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 14:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 15);
		break;

	case 15:
		compact_mem_and_report();
		kernel_timing_trigger(6, 16);
		break;

	case 16:
		digi_unload_stream_breaks(SERIES4);
		series_stream_with_breaks(SERIES6, "702B", 6, 1, 17);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 17:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 18);
		break;

	case 18:
		compact_mem_and_report();
		kernel_timing_trigger(6, 19);
		break;

	case 19:
		digi_unload_stream_breaks(SERIES5);
		digi_unload_stream_breaks(SERIES6);
		digi_preload_stream_breaks(SERIES7);
		series_stream_with_breaks(SERIES7, "704A", 6, 1, 20);
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 20:
		pal_fade_set_start(0);
		kernel_timing_trigger(6, 21);
		break;

	case 21:
		compact_mem_and_report();
		kernel_timing_trigger(6, 26);
		break;

	case 22:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 60, -1);
		break;

	case 23:
		pal_mirror_colours(119, 121);
		gr_pal_set_range(120, 8);
		pal_cycle_init(119, 124, 10, -1, -1);
		break;

	case 24:
		pal_mirror_colours(120, 122);
		gr_pal_set_range(120, 8);
		pal_cycle_init(120, 122, 10, -1, -1);
		break;

	case 25:
		inv_give_to_player("MIRROR");
		break;

	case 26:
		_G(game).new_room = 702;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
