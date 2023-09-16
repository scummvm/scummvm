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

#include "m4/burger/rooms/section4/room405.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room405::SAID[][4] = {
	{ "VIPE",          nullptr,   "400W001", "400W001" },
	{ "GUITAR",        "405w003", nullptr,   nullptr   },
	{ "RECORDS",       "405w004", nullptr,   nullptr   },
	{ "VERA",          "405w006", "400w001", "400w001" },
	{ "JUKEBOX",       "405w007", nullptr,   nullptr   },
	{ "DISC",          "400w002", "400w003", nullptr   },
	{ "DISC ",         "400w002", "400w003", nullptr   },
	{ "SWINGING DOOR", "405w009", "400w001", nullptr   },
	{ "ORDER WINDOW",  "405w009", "400w001", "400w001" },
	{ "MUFFIN",        "405w010", nullptr,   nullptr   },
	{ "FOUNTAIN",      "405W011", nullptr,   nullptr   },
	{ "MOOSEHEAD",     "405W012", nullptr,   nullptr   },
	{ "MENU",          "405W013", nullptr,   nullptr   },
	{ "STOOL",         nullptr,   nullptr,   "405W019" },
	{ "BOOTH",         nullptr,   nullptr,   "405W019" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room405::SERIES1[] = {
	{  8,  nullptr,   0,   0, 22, 0, nullptr, 0 },
	{  15, "405v512", 1, 255, -1, 0, nullptr, 0 },
	{ 115, "405_008", 2, 255, -1, 0, nullptr, 0 },
	{ 123, "405e503", 1, 255, 13, 0, nullptr, 0 },
	{ 210, "999_003", 2, 255, -1, 0, nullptr, 0 },
	{ 221, "999_003", 2, 255, -1, 0, nullptr, 0 },
	{ 235, nullptr,   2, 255,  7, 0, nullptr, 0 },
	{ 250, nullptr,   0,   0, 29, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room405::PLAY1[] = {
	{ 0, 3, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY2[] = {
	{  0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{  2,  9, nullptr, 0, 0, 11, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr, 0, 0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY3[] = {
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

const seriesPlayBreak Room405::PLAY4[] = {
	{ 0, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY5[] = {
	{  0,  5, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  6, 42, "405_006", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 43, 51, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 60, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 61, 69, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 70, 78, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 79, 85, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 86, -1, "405_004", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY7[] = {
	{ 0, -1, "405_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room405::init() {
}

void Room405::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
