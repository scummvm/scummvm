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

#include "m4/burger/rooms/section3/room310.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room310::SAID[][4] = {
	{ "TUNNEL",   "310w002", "310w003",  nullptr   },
	{ "PROBE",    "310w004",  nullptr,   nullptr   },
	{ "TRUFFLES", nullptr,    "310w011", "310w012" },
	{ "GROUND",   "310w017",  "310w003", "310w003" },
	{ "WALL",     "310w017",  "310w003", "310w003" },
	{ "CEILING",  "310w017",  "310w003", "310w003" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room310::PLAY1[] = {
	{ 1,  2, "300t004a", 2, 165, -1, 0, 0, &_state1, 1 },
	{ 1,  2, "300t004b", 2, 165, -1, 0, 0, &_state1, 2 },
	{ 1,  2, "300t004c", 2, 165, -1, 0, 0, &_state1, 3 },
	{ 1,  2, "300t004d", 2, 165, -1, 0, 0, &_state1, 4 },
	{ 3,  4, nullptr,    2,   0,  1, 0, 0, nullptr,  0 },
	{ 5, -1, nullptr,    2,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room310::PLAY2[] = {
	{  0,  9, "300t003a", 2, 165,    -1,    0, 0, &_state1, 1 },
	{  0,  9, "300t003b", 2, 165,    -1,    0, 0, &_state1, 2 },
	{  0,  9, "300t003c", 2, 165,    -1,    0, 0, &_state1, 3 },
	{  0,  9, "300t003d", 2, 165,    -1,    0, 0, &_state1, 4 },
	{ 10, 16, nullptr,    2,   0, 10016,    1, 0, nullptr,  0 },
	{ 10, 16, nullptr,    2,   0,    -1,    0, 0, nullptr,  0 },
	{ 17, 21, nullptr,    2,   0,    -1,    0, 0, nullptr,  0 },
	{  0,  0, nullptr,    2,   0,    -1, 2048, 0, nullptr,  0 },
};

const seriesPlayBreak Room310::PLAY3[] = {
	{ 15, 29, nullptr, 2, 0, -1, 0, 0, nullptr, 0 },
};

const seriesPlayBreak Room310::PLAY4[] = {
	{ 6, -1, nullptr, 1, 0, -1, 0, 0, nullptr, 0 },
};

const seriesPlayBreak Room310::PLAY5[] = {
	{ 30, 36, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 37, 51, "300_002", 2, 255, -1, 0, 0, nullptr, 0 },
};

const seriesPlayBreak Room310::PLAY6[] = {
	{ 52, 56, "300_001", 2, 255, -1, 0, 0, nullptr, 0 },
};

long Room310::_state1;


Room310::Room310() : Section3Room() {
	_state1 = 0;
}

void Room310::init() {
}

void Room310::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
