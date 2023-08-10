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

static const seriesStreamBreak STREAMS1[] = {
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
		digi_preload_stream_breaks(STREAMS1);
		kernel_trigger_dispatch_now(14);
	} else {
		kernel_trigger_dispatch_now(1);
	}
}

void Room802::daemon() {
	warning("TODO: room 802 daemon");
}

void Room802::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv81")) {
		kernel_trigger_dispatch_now(19);
		_G(player).command_ready = false;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
