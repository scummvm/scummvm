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

#include "m4/burger/rooms/section4/room404.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room404::SAID[][4] = {
	{ "QUARTER ",     "404w001", nullptr,   "404w002" },
	{ "IMPOUND YARD", nullptr,   "400w001", nullptr   },
	{ "TOILET",       "404w003", "400w001", "404w004" },
	{ "SINK",         "404w005", "400w001", "404w006" },
	{ "COT",          "404w007", nullptr,   "404w008" },
	{ "MARKS",        "404w103", nullptr,   nullptr   },
	{ "NEWSPAPER",    "404w009", "404w010", "404w011" },
	{ "BARS",         "404w012", "400w001", "400w001" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room404::PLAY1[] = {
	{  1,  9, nullptr, 0, 0,    -1, 0, 0, nullptr, 0 },
	{ 10, 17, nullptr, 0, 0, 10016, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room404::init() {
}

void Room404::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
