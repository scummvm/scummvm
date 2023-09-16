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

#include "m4/burger/rooms/section4/room402.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room402::SAID[][4] = {
	{ "bench",       "402w006", nullptr,   "402w007" },
	{ "force field", "400w005", nullptr,   "400w001" },
	{ "sidewalk",    nullptr,   nullptr,   nullptr   },
	{ "steps",       nullptr,   nullptr,   nullptr   },
	{ "road",        nullptr,   nullptr,   nullptr   },
	{ "townhall",    nullptr,   nullptr,   nullptr   },
	{ "sky",         nullptr,   nullptr,   nullptr   },
	{ "bush",        nullptr,   nullptr,   nullptr   },
	{ "steps",       nullptr,   nullptr,   nullptr   },
	{ "pillar",      nullptr,   nullptr,   nullptr   },
	{ "window",      nullptr,   nullptr,   nullptr   },
	{ "bushes",      nullptr,   nullptr,   nullptr   },
	{ "stolie",      nullptr,   "400w001", "400w001" },
	{ "elmo",        "402w003", "400w001", "400w001" },
	{ "disc",        nullptr,   "400w003", nullptr   },
	{ "disc ",       nullptr,   "400w003", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room402::SERIES1[] = {
	{ 0, "402s001", 1, 255, -1, 0, nullptr, 0 },
	{ 21, nullptr,  1, 255,  7, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room402::SERIES2[] = {
	{  0, "402s003", 1, 255, -1, 0, nullptr, 0 },
	{  4, "402_002", 2, 255, -1, 0, nullptr, 0 },
	{ 11, nullptr,   1, 255,  9, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room402::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY2[] = {
	{  0,  5, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{  6,  9, "402s002", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 10, 11, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY3[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY4[] = {
	{ 2, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY5[] = {
	{ 0, 4, "402p901", 1, 255, -1, 4, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY6[] = {
	{ 10, 16, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY7[] = {
	{ 18, 21, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY8[] = {
	{ 2, 10, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY9[] = {
	{ 0, 2, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 3, 5, "402s004", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 6, 13, nullptr,  0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY10[] = {
	{  0,  1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{  2,  9, nullptr,   0,   0, 38,    0, 0, nullptr, 0 },
	{ 10, 11, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 12, 12, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 12, 12, "402_001", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 14, 16, nullptr,   2,   0, -1, 2048, 0, nullptr, 0 },
	{  3,  2, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY11[] = {
	{ 19, 17, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 16, 16, nullptr, 0, 0, 33, 0, 0, nullptr, 0 },
	{ 15, 12, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{  7,  2, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 22, 23, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY12[] = {
	{ 0, 9, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY13[] = {
	{ 0,  2, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 3,  6, "402S005", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 7, 13, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY14[] = {
	{  0,  1, "999_003", 1, 255, -1, 0, 0, nullptr, 0 },
	{  2,  2, nullptr,   0,   0,  4, 0, 0, nullptr, 0 },
	{  3,  9, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 10, 11, "999_003", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 12, -1, nullptr,   0,   0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY15[] = {
	{ 0, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY16[] = {
	{  0, 16, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 17, 20, "402_001",  1, 255,   -1, 0, 0, nullptr, 0 },
	{ 20, 20, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 20, 20, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 20, 20, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 21, 23, nullptr,    0,   0, 2048, 0, 0, nullptr, 0 },
	{ 24, 36, "402p903a", 1, 255,   -1, 0, 0, nullptr, 0 },
	{ 64, 66, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room402::init() {
}

void Room402::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
