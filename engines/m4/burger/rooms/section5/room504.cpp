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

#include "m4/burger/rooms/section5/room504.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room504::SAID[][4] = {
	{ "WILBUR'S ROOM", nullptr,   "500w001", nullptr },
	{ "BATHROOM",      nullptr,   "500w001", nullptr },
	{ "AUNT POLLY'S ROOM", nullptr, "500w001", nullptr },
	{ "STAIRS",        nullptr,   "505w001", "500w001" },
	{ "STAIRWAY BORK", "505w003", "500w002", "500w002" },
	{ "RAILING",       "505w004", nullptr,   "505w006" },
	{ "WINDOW",        "500w003", nullptr,   "500w004" },
	{ "CEILING FAN",   "505w008", nullptr,   nullptr   },
	{ "CHANDELIER",    "505w009", nullptr,   nullptr   },
	{ "VASE",          "505w010", "505w011", "505w012" },
	{ "PICTURE",       "505w013", "500w005", "505w014" },
	{ "PICTURE ",      "505w015", "500w005", "505w014" },
	{ "PICTURE  ",     "505w016", "500w005", "505w014" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room504::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY3[] = {
	{ 0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 8,  8, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, -1, nullptr,   0,   0,  8, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY4[] = {
	{  0,  8, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  9,  9, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr,   0,   0,  8, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY5[] = {
	{ 0,  5, nullptr, 0, 0,    -1, 1, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 0, 10016, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY6[] = {
	{ 0,  4, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 5,  5, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr,   0,   0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY7[] = {
	{ 0,  6, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 7,  7, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, -1, nullptr,   0,   0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY8[] = {
	{ 0,  6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, -1, nullptr, 0, 0, 10, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY9[] = {
	{  0, 30, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 31, -1, nullptr, 0, 0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY10[] = {
	{  0, 12, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 13, 13, "504w004",  1, 255, -1, 0, 0, &_state1, 1 },
	{ 13, 13, "504w018a", 1, 255, -1, 0, 0, &_state1, 2 },
	{ 13, 13, "504w018b", 1, 255, -1, 0, 0, &_state1, 3 },
	{ 13, 13, "504w018c", 1, 255, -1, 0, 0, &_state1, 4 },
	{ 13, 13, "504w018d", 1, 255, -1, 0, 0, &_state1, 5 },
	{ 13, 13, "504w018e", 1, 255, -1, 0, 0, &_state1, 6 },
	{ 14, 14, nullptr,    0,   0, -1, 0, 5, nullptr,  0 },
	{ 15, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY11[] = {
	{  0,  3, nullptr,    0,   0,  6, 0,  0, nullptr,  0 },
	{  6, 15, "504b002a", 2, 255, -1, 0,  0, &_state2, 1 },
	{  6, 15, "504b002b", 2, 255, -1, 0,  0, &_state2, 2 },
	{  6, 15, "504b002c", 2, 255, -1, 0,  0, &_state2, 3 },
	{ 16, 17, "504_005",  3, 125, -1, 0,  0, nullptr,  0 },
	{ 18, 27, "504b003",  2, 255, -1, 0,  0, nullptr,  0 },
	{ 28, 30, "504_006",  3, 125, -1, 0,  0, nullptr,  0 },
	{ 31, 41, "504b013a", 2, 255, -1, 1, -1, nullptr,  0 },
	{ 29, 33, "504b013c", 2, 255, -1, 1, -1, nullptr,  0 },
	{ 42, 47, "504_004",  3, 125, -1, 0,  0, nullptr,  0 },
	{ 49, 55, "504b005a", 2, 255, -1, 0,  0, &_state3, 1 },
	{ 49, 55, "504b005b", 2, 255, -1, 0,  0, &_state3, 2 },
	{ 56, 75, "504b006a", 2, 255, -1, 0,  0, &_state4, 1 },
	{ 56, 75, "504b006b", 2, 255, -1, 0,  0, &_state4, 2 },
	{ 76, 81, "500_014",  2, 255, -1, 0,  0, nullptr,  0 },
	{ 82, 82, nullptr,    0,   0,  6, 0,  7, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY12[] = {
	{ 0, -1, "504b001a", 2, 255, -1, 1, 0, &_state2, 1 },
	{ 0, -1, "504b001b", 2, 255, -1, 1, 0, &_state2, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY13[] = {
	{  0,  8, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  9,  9, "504_005",  1, 125, -1, 0, 0, nullptr,  0 },
	{ 10, 19, "504b008a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 10, 19, "504b008b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 10, 19, "504b008c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 20, 20, "504_007",  3, 255, -1, 0, 0, nullptr,  0 },
	{ 21, 24, "504_006",  1, 125, -1, 0, 0, nullptr,  0 },
	{ 25, 27, "504b012a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 25, 27, "504b012b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 28, 29, "504_002",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 30, -1, "504_004",  1, 125, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY14[] = {
	{ 0, -1, "504b008a",  2, 255, -1, 0, 0, &_state2, 1 },
	{ 0, -1,  "504b008b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "504b008c",  2, 255, -1, 0, 0, &_state2, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY15[] = {
	{ 0, -1, "504_005", 3, 125, -1, 0, 2, nullptr, 0 },
	{ 0, -1, "504_006", 3, 125, -1, 0, 0, nullptr, 0 },
	{ 0, -1, "504_004", 3, 125, -1, 0, 4, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY16[] = {
	{ 0, -1, "500_008", 2, 255, -1, 0, 0, &_state5, 1 },
	{ 0, -1, "500_009", 2, 255, -1, 0, 0, &_state5, 2 },
	{ 0, -1, "500_010", 2, 255, -1, 0, 0, &_state5, 3 },
	{ 0, -1, "500_011", 2, 255, -1, 0, 0, &_state5, 4 },
	{ 0, -1, "500_012", 2, 255, -1, 0, 0, &_state5, 5 },
	{ 0, -1, "500_013", 2, 255, -1, 0, 0, &_state5, 6 },
	PLAY_BREAK_END
};

long Room504::_state1;
long Room504::_state2;
long Room504::_state3;
long Room504::_state4;
long Room504::_state5;

Room504::Room504() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
	_state5 = 0;
}

void Room504::init() {
}

void Room504::daemon() {
}

void Room504::pre_parser() {

}

void Room504::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
