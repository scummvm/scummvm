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
	pal_fade_set_start(_G(master_palette), 0);
	pal_fade_init(_G(master_palette), 0, 255, 100, 40, -1);

	_val1 = 11;
	_val2 = 2;
	_val3 = 0;
	_val4 = 0;

	if (_G(flags)[NEURO_TEST_COUNTER] > 1) {
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

	// TODO: More cases
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

} // namespace Rooms
} // namespace Burger
} // namespace M4
