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

#include "m4/burger/rooms/section1/room143.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room143::SAID[][4] = {
	{ "VERA",          "143W001", "999w011", "999w011" },
	{ "BURL",          nullptr,   "999w011", "999w011" },
	{ "SWINGING DOOR", "143w006", "999w011", nullptr   },
	{ "ORDER WINDOW",  "143w006", "999w011", "999w011" },
	{ "JUKEBOX",       "143w008", nullptr,   nullptr   },
	{ "OUTSIDE",       nullptr,   "999w011", nullptr   },
	{ "MUFFIN",        "143w009", nullptr,   nullptr   },
	{ "FOUNTAIN",      "143W010", nullptr,   nullptr   },
	{ "MOOSEHEAD",     "143W011", nullptr,   nullptr   },
	{ "MENU",          "143W012", nullptr,   nullptr   },
	{ "MOUSE TRAP",    "143W013", nullptr,   "999w011" },
	{ "STOOL",         nullptr,   nullptr,   "143W017" },
	{ "BOOTH",         nullptr,   nullptr,   "143W017" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room143::PLAY1[] = {
	{  0, 20, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 21, 21, "405_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 22, 28, "405w015", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, -1, "143_008", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY2[] = {
	{ 0,  5, nullptr, 0,   0, -1, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 255, 12, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY3[] = {
	{  0, 76, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 77, -1, nullptr, 2, 0,  3, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY4[] = {
	{ 0, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY5[] = {
	{ 0, 3, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY6[] = {
	{  0,  2, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  3, 14, "143_006", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 15, 25, "143_005", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 26, 29, "143_007", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 30, 30, nullptr,   0,   0, 40, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY7[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY8[] = {
	{  0, 22, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 24, -1, nullptr, 0, 0,  8, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY9[] = {
	{  0, 25, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 26, -1, "143_004", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY10[] = {
	{  0,  6, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{  6,  6, nullptr,   0,   0, 32,    0, 0, nullptr, 0 },
	{  7, 12, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 16, "143b007", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 17, 21, "143b009", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 22, 23, nullptr,   1, 255, -1, 2048, 0, nullptr, 0 },
	{ 24, -1, "143b010", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room143::PLAY11[] = {
	{ 0, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, 1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, 0, nullptr, 0, 0, 36, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room143::init() {

}

void Room143::daemon() {

}

void Room143::pre_parser() {

}

void Room143::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
