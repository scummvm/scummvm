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

#include "m4/burger/rooms/section1/room175.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/core/play_break.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "WILBUR'S ROOM",     nullptr,   "175w001", nullptr   },
	{ "BATHROOM",          nullptr,   "175w001", nullptr   },
	{ "AUNT POLLY'S ROOM", "175w002", "175w001", "175w003" },
	{ "STAIRS",            "175w004", "175w001", nullptr   },
	{ "WINDOW",            nullptr,   nullptr,   "175w005" },
	{ "VASE",              "175w006", "175w007", "175w008" },
	{ "PICTURE",           "175w012", "175w010", "175w011" },
	{ "PICTURE ",          "175w013", "175w010", "175w011" },
	{ "PICTURE  ",         "175w014", "175w010", "175w011" },
	{ "PICTURE   ",        "175w009", "176w010", "175w011" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0,  3, "175_002a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 4, -1, "175_002b", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0,  2, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{ 3, -1, "175_002a", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};


void Room175::init() {
}

void Room175::daemon() {
}

void Room175::pre_parser() {
}

void Room175::parser() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
