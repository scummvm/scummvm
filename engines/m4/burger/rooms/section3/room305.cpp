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

#include "m4/burger/rooms/section3/room305.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room305::SAID[][4] = {
	{ "STAIRS",      "305w002", "300w002", nullptr   },
	{ "DARKNESS",    "305w004", "300w002", "300w002" },
	{ "MINE",        "305w003", "300w002", "300w002" },
	{ "GENERATOR",   nullptr,   "305w008", nullptr   },
	{ "PICK AXE",    "305w015", "305w016", "305w017" },
	{ "BOX",         "305w018", "300w003", "300w005" },
	{ "DEBRIS",      nullptr,   "305w021", "300w005" },
	{ "GROUND",      "305w022", "300w002", "300w002" },
	{ "WALL",        "305w022", "300w002", "300w002" },
	{ "CEILING",     "305w022", "300w002", "300w002" },
	{ "FORCE FIELD", "302w027", "300w002", "300w002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room305::SERIES1[] = {
	{  6, "305_003", 2, 255, -1,    0, nullptr, 0 },
	{ 11, "305w009", 1, 255, -1,    0, nullptr, 0 },
	{ 12, "305_005", 2, 255, -1,    0, nullptr, 0 },
	{ 20, "305w010", 1, 255, -1,    0, &Flags::_flags[V135], 0 },
	{ 30, "305_006", 2, 255, -1, (uint)-1, nullptr, 0 },
	{ 72, "305_001", 1, 255, -1,    0, nullptr, 0 },
	{ 72, nullptr,   2,   0, -1, 2048, nullptr, 0 },
	{ 79, "305_004", 2, 255, -1,    0, nullptr, 0 },
	{ 94, "305_002", 2, 255, -1,    0, nullptr, 0 },
	{ -1, 0, 0, 0, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room305::PLAY1[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room305::PLAY2[] = {
	{ 0, -1, 0, 1, 0, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};


void Room305::init() {
	setupDigi();
	set_palette_brightness(60);
	pal_cycle_init(96, 111, 6, -1, -1);
	_G(flags)[V149] = 0;

	if (_G(flags)[V134] && !_G(flags)[V135] && !_G(flags)[V137]) {
		hotspot_set_active("DARKNESS", true);
		hotspot_set_active("MINE", false);
	} else {
		hotspot_set_active("DARKNESS", false);
		hotspot_set_active("MINE", true);
	}

	_series1 = series_show(_G(flags)[V134] ? "305genx" : "305gen", 0x200);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 302:
		_G(wilbur_should) = 101;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	case 319:
		player_set_commands_allowed(true);
		ws_demand_location(-50, 200);
		kernel_trigger_dispatch_now(301);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(320, 290, 7);
		break;
	}
}

void Room305::daemon() {
}

void Room305::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
