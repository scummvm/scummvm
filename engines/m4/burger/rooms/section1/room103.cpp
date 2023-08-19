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
	_G(player).walker_in_this_scene = true;
	_val1 = 0;
	_val2 = 0;
	_val3 = 0;

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		break;

	case 101:
		ws_demand_location(327, 178, 7);
		_G(flags)[V012] = 0;

		player_set_commands_allowed(false);
		ws_hide_walker();
		_G(roomVal1) = 1;
		digi_preload_stream_breaks(SERIES1);
		series_play("103wi01s", 0x100);
		series_stream_with_breaks(SERIES1, "103wi01", 6, 0xa00, 10016);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	if (_G(flags)[V023]) {
		_series1 = series_play("103wi06", 0x500, 0, -1, 100);
	} else {
		_series1 = series_play("103wi05", 0x500, 0, -1, 100);
	}

	_series2 = series_play("103door", 0x800, 0, -1, 100);
	series_play("103vent", 0x100);

	_val4 = 12;
	kernel_trigger_dispatch_now(10);
	_val5 = 19;
	kernel_trigger_dispatch_now(12);
	_val6 = 21;
	kernel_trigger_dispatch_now(13);

	digi_play_loop("103_001", 3, 60);
}

void Room103::daemon() {
	// TODO
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
		if (player_said("gear", "satellite dish") && _G(flags)[V023] == 0) {
			_G(roomVal1) = 4;
			kernel_trigger_dispatch_now(gTELEPORT);
			player_set_commands_allowed(false);
			_G(player).command_ready = false;

		} else if (player_said("gear", "door") || player_said("gear", "stairway")) {
			_G(roomVal1) = 2;
			kernel_trigger_dispatch_now(gTELEPORT);
			player_set_commands_allowed(false);
			_G(player).command_ready = false;

		} else if (player_said("ENTER", "FIRE ESCAPE") || player_said("gear", "FIRE ESCAPE")) {
			_val2 = 1;
			_G(roomVal1) = 6;
			kernel_trigger_dispatch_now(gTELEPORT);
			player_set_commands_allowed(false);
			_G(player).command_ready = false;
		}
	} else {
		_G(player).command_ready = false;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
