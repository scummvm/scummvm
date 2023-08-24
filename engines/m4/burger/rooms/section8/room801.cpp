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

static const seriesStreamBreak STREAMS1[] = {
	{ 0, "805_001", 3,  80, -1, 0, nullptr, 0 },
	{ 1, "805w001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS2[] = {
	{  1, "806f002", 1, 255, -1, 0, nullptr, 0 },
	{ 22, "806z002", 1, 255, -1, 0, nullptr, 0 },
	{ 36, "806f003", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS3[] = {
	{   0, "803w001", 1, 255, -1, 0, nullptr, 0 },
	{  59, "803z006", 1, 255, -1, 0, nullptr, 0 },
	{  86, "803z007", 1, 255, -1, 0, nullptr, 0 },
	{ 116, "803z008", 1, 255, -1, 0, nullptr, 0 },
	{ 182, "803f005", 1, 255, -1, 0, nullptr, 0 },
	{ 220, "803z009", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS4[] = {
	{  5, "802_014", 2, 255, -1, 0, nullptr, 0 },
	{ 16, "802f008", 1, 255, -1, 0, nullptr, 0 },
	{ 28, "802f009", 1, 255, -1, 0, nullptr, 0 },
	{ 87, "802_015", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS5[] = {
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

static const seriesStreamBreak STREAMS6[] = {
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

static const seriesStreamBreak STREAMS7[] = {
	{   0, nullptr,   0,   0, 37, 0, nullptr, 0 },
	{   7, "801_003", 1, 175, -1, 0, nullptr, 0 },
	{  23, "802_017", 2,  75, -1, 0, nullptr, 0 },
	{  54, nullptr,   0,   0, 41, 0, nullptr, 0 },
	{  61, "802f001", 1, 255, -1, 0, nullptr, 0 },
	{ 115, "802w002", 2, 255, -1, 0, nullptr, 0 },
	{ 119, "802f002", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS8[] = {
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

static const seriesStreamBreak STREAMS9[] = {
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
	pal_fade_set_start(_G(master_palette), 0);
	pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 0, -1);
	_val1 = 10;
	digi_preload("800_001");

	if (_G(flags)[GLB_TEMP_5] == 1) {
		term_message(HEADER);
		term_message("Toxic wax in hair");

		digi_preload_stream_breaks(STREAMS1);
		digi_preload_stream_breaks(STREAMS2);
		digi_preload_stream_breaks(STREAMS3);
		digi_preload_stream_breaks(STREAMS4);
		digi_preload_stream_breaks(STREAMS5);

		kernel_trigger_dispatch_now(20);
	} else {
		switch (_G(flags)[NEURO_TEST_COUNTER]) {
		case 0:
			term_message(HEADER);
			term_message("FIRST ATTEMPT AT NEURO TEST");

			digi_preload_stream_breaks(STREAMS6);
			digi_preload_stream_breaks(STREAMS7);
			digi_preload_stream_breaks(STREAMS8);

			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 0, -1);
			kernel_trigger_dispatch_now(1);
			break;

		case 1:
			term_message(HEADER);
			term_message("SECOND ATTEMPT AT NEURO TEST");
			digi_preload_stream_breaks(STREAMS8);
			kernel_trigger_dispatch_now(4);
			break;

		case 2:
			term_message(HEADER);
			term_message("THIRD ATTEMPT AT NEURO TEST");
			digi_preload_stream_breaks(STREAMS8);
			kernel_trigger_dispatch_now(41);
			kernel_trigger_dispatch_now(7);
			break;

		default:
			term_message(HEADER);
			term_message("FOURTH OR GREATER ATTEMPT AT NEURO TEST");
			digi_preload_stream_breaks(STREAMS5);
			kernel_trigger_dispatch_now(20);
			break;
		}
	}
}

void Room801::daemon() {
	warning("TODO: Room 801 daemon");
}

void Room801::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv80")) {
		kernel_trigger_dispatch_now(14);
		_G(player).command_ready = false;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
