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

#include "m4/burger/rooms/section1/room174.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "HALLWAY",     nullptr,   "174w001", nullptr   },
	{ "TOILET",      "174w002", "174w001", nullptr   },
	{ "TOILET LID",  "174w002", "174w001", nullptr   },
	{ "TOILET TANK", "174w002", "174w001", nullptr   },
	{ "FLUSH CHAIN", "174w002", "174w001", nullptr   },
	{ "TUB",         "174w004", "174w001", "174w005" },
	{ "WINDOW",      nullptr,   "174w001", "174w006" },
	{ "SHOWERHEAD",  "174w007", "174w008", nullptr   },
	{ "CLOSET",      "174w009", "174w010", "174w010" },
	{ "MIRROR",      "174w011", nullptr,   "174w011" },
	{ "LAXATIVE ",   "174w012", nullptr,   "174w014" },
	{ "MEDICATIONS", "174w015", "174w016", "174w017" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0,  1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0, 18, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0,  3, "507_003", 2, 255, -1, 1, 5, nullptr, 0 },
	{ 4, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 6,  6, nullptr,   0,   0, -1, 0, 5, nullptr, 0 },
	{ 5,  5, nullptr,   0,   0, -1, 0, 5, nullptr, 0 },
	{ 4,  4, nullptr,   0,   0, -1, 0, 5, nullptr, 0 },
	PLAY_BREAK_END
};


void Room174::init() {
}

void Room174::daemon() {
}

void Room174::pre_parser() {
}

void Room174::parser() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
