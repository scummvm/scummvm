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

#include "m4/burger/rooms/section3/room302.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room302::SAID[][4] = {
	{ "DOOR",           nullptr,   "300w001", nullptr   },
	{ "STORM CELLAR",   nullptr,   "300w002", nullptr   },
	{ "BACK YARD",      "302w002", "302w002", nullptr   },
	{ "BURGER MORSEL ", nullptr,   nullptr,   "302w010" },
	{ "TRUFFLES",       nullptr,   "302w013", "302w014" },
	{ "TROUGH",         "302w015", "300w003", "302w016" },
	{ "SIGN",           "302w017", "300w004", "300w005" },
	{ "CABIN",          "302w018", "300w002", "300w002" },
	{ "WINDOW",         "302w021", "300w002", "302w022" },
	{ "ROCKING CHAIR",  "302w019", "302w020", "302w020" },
	{ "CRASHED ROCKET", "302w023", "300w003", "302w024" },
	{ "DOCK",           "302w025", "300w002", "302w026" },
	{ "FORCE FIELD",    "302w027", "300w002", "300w002" },
	{ "ROCK",           "302w028", "300w002", "300w002" },
	{ "TREES",          "302w029", "300w002", "300w002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room302::SERIES1[] = {
	{  0, "302p004", 1, 255, -1, 0, nullptr, 0 },
	{ 19, "302_008", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room302::SERIES2[] = {
	{   7, "302_002",  1, 255, -1,    0, nullptr,  0 },
	{   9, "302w001a", 2, 255, -1,    0, &_state1, 0 },
	{   9, "302w001b", 2, 255, -1,    0, &_state1, 1 },
	{   9, "302w001c", 2, 255, -1,    0, &_state1, 2 },
	{  34, "302_005",  2, 125, -1,    0, nullptr,  0 },
	{  48, "302_006",  2, 125, -1, 1024, nullptr,  0 },
	{ 138, "302_007",  1, 255, -1,    0, nullptr,  0 },
	{ 138, "302_003",  1, 255, -1,    0, nullptr,  0 },
	{ 152, "302_007",  1, 255, -1,    0, nullptr,  0 },
	{ 152, "302_003",  1, 255, -1,    0, nullptr,  0 },
	{ 165, nullptr,    2,   0,  4,    0, nullptr,  0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room302::SERIES3[] = {
	{  7, "302_002",  2, 255, -1,    0, nullptr,  0 },
	{  9, "302w001a", 1, 255, -1,    0, &_state1, 0 },
	{  9, "302w001b", 1, 255, -1,    0, &_state1, 1 },
	{  9, "302w001c", 1, 255, -1,    0, &_state1, 2 },
	{ 24, "302p001",  2, 255, -1,    0, nullptr,  0 },
	{ 35, "302_005",  2, 125, -1,    0, nullptr,  0 },
	{ 48, "302_006",  2, 125, -1, 1024, nullptr,  0 },
	{ 52, "302p003",  1, 255, -1,    0, nullptr,  0 },
	{ 71, "302_008",  2, 255, -1,    0, nullptr,  0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room302::PLAY1[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY2[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY3[] = {
	{ 0, -1, "304_001", 2, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY4[] = {
	{ 0,  1, nullptr,   1,   0, -1, 0, 0, nullptr, 0 },
	{ 2, -1, "302w002", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY5[] = {
	{  0, 22, nullptr,   1,   0, -1, 0, 0, nullptr, 0 },
	{ 23, -1, "302_001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY6[] = {
	{ 0,  7, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	{ 8, -1, nullptr, 1, 0,  5,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY7[] = {
	{  0, 29, "302t004", 2, 155, -1, 2048,  0, nullptr, 0 },
	{ 26, 29, "302t005", 2, 155, -1,    0, 12, &Flags::_flags[V110], 0 },
	{ 26, 29, "302t005", 2, 155, -1,    0,  3, &Flags::_flags[V110], 1 },
	{ 30, 61, nullptr,   2,   0, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY8[] = {
	{ 62, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY9[] = {
	{  0, 28, nullptr, 1, 0, -1, 0, 0, nullptr, 0 },
	{ 29, -1, nullptr, 1, 0, 6, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY10[] = {
	{ 0, 4, nullptr, 2, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY11[] = {
	{ 5, 13, nullptr, 2, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY12[] = {
	{  6, 16, "302t002", 2, 155, -1, 0, 0, nullptr, 0 },
	{ 11, 17, nullptr,   2,   0, -1, 3, 2, nullptr, 0 },
	{ 12,  6, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY13[] = {
	{ 0, 4, "302t003", 2, 155, -1, 0, 0, nullptr, 0 },
	{ 5, 3, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 5, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 3, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 3, 5, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 5, 3, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 3, 4, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 5, 0, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

long Room302::_state1;


Room302::Room302() : Section3Room() {
	_state1 = 0;
}

void Room302::init() {
}

void Room302::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
