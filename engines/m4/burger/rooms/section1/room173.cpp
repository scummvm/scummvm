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

#include "m4/burger/rooms/section1/room173.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "STAIRS",          "173w002", "173w003", nullptr   },
	{ "WRINGER",         "173w009", "173w010", "173w008" },
	{ "BREAKER BOX",     "504w017", nullptr,   nullptr   },
	{ "WASHING MACHINE", "173w007", nullptr,   "173w008" },
	{ "SHORTS",          "173w004", "173w005", "173w005" },
	{ "FAN BOY TOWEL",   "173w004", "173w005", "173w005" },
	{ "SOCK",            "173w004", "173w005", "173w005" },
	{ "T-SHIRT",         "173w004", "173w005", "173w005" },
	{ "LAUNDRY",         "173w004", "173w006", "173w006" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0,  5, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 0,  1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room173::init() {
}

void Room173::daemon() {
}

void Room173::pre_parser() {
}

void Room173::parser() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
