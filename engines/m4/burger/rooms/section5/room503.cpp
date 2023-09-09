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

#include "m4/burger/rooms/section5/room503.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room503::SAID1[][4] = {
	{ "PARLOUR",        nullptr,   "500w001", nullptr   },
	{ "BASEMENT",       nullptr,   "500w001", nullptr   },
	{ "BORK",           "503w004", "500w002", "500w002" },
	{ "ROLLING PIN ",   "503w007", nullptr,   "503w008" },
	{ "MICROWAVE",      "503w009", nullptr,   nullptr   },
	{ "RUBBER GLOVES ", "503w014", nullptr,   nullptr   },
	{ "HUTCH",          "503w016", nullptr,   "500w001" },
	{ "DISHES",         "503w017", "503w018", "503w018" },
	{ "FRIDGE",         "503w019", nullptr,   "500w001" },
	{ "OVEN",           "503w020", nullptr,   nullptr   },
	{ "MOP",            "503w024", "503w025", "503w025" },
	{ "CUPBOARD ",      "503w026", nullptr,   nullptr   },
	{ "CUPBOARD",       "503w028", nullptr,   nullptr   },
	{ "PRUNES",         "503w029", "503w031", "503w031" },
	{ "WINDOW",         "500w003", nullptr,   "500w004" },
	{ "SINK",           "503w034", "503w035", nullptr   },
	{ "OUTLET",         "503w036", "500w001", "503w037" },
	{ "CALENDAR",       "503w038", "500w005", "503w038" },
	{ "TOASTER",        "503w039", "503w040", "503w040" },
	{ "COOKIES",        "503w041", "503w042", "503w043" },
	{ "WIRES",          "503w044", "503w045", "503w045" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const char *Room503::SAID2[][4] = {
	{ "STAIRS",         "504w002", "500w001", nullptr   },
	{ "BORK",           "504w007", "500w002", "500w002" },
	{ "BORK GUTS",      "504w009", "504w010", "504w010" },
	{ "DIRTY SOCK ",    "504w011", nullptr,   nullptr   },
	{ "WRINGER",        "504w013", "504w015", "504w016" },
	{ "BREAKER BOX",    "504w017", nullptr,   "504w019" },
	{ "INSIDE BREAKER", "504w020", "504w021", nullptr   },
	{ "OUTSIDE BREAKER","504w024", "504w021", nullptr   },
	{ "WASHING MACHINE","504w026", nullptr,   "504w027" },
	{ "PAINT CAN",      "504w028", "504w029", "504w030" },
	{ "HOLE",           nullptr,   "500w001", "500w001" },
	{ "LAUNDRY HAMPER", "504w030", "504w016", "504w016" },
	{ "LAUNDRY HAMPER ","504w030", "504w016", "504w016" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room503::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY2[] = {
	{  0,  2, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  3, 18, "503b001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 19, -1, "503_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY3[] = {
	{  0,  3, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  4,  9, "503_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 10, 14, "503_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 15, 18, "503_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 19, -1, "503_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY4[] = {
	{  0,  4, "503b004a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0,  4, "503b004b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  0,  4, "503b004c", 2, 255, -1, 0, 0, &_state1, 3 },
	{  5,  9, nullptr,    0,   0, 13, 0, 0, nullptr,  0 },
	{ 10, 10, nullptr,    0,   0, 18, 0, 0, &_state2, 3 },
	{ 10, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY5[] = {
	{ 0, -1, "503b005a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 0, -1, "503b005b", 2, 255, -1, 0, 0, &_state1, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY6[] = {
	{  0,  3, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  4,  7, nullptr,    0,   0, 10, 0, 0, nullptr,  0 },
	{  8, 10, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 11, 13, "503b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 13, "503b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 14, 20, "503b007a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 14, 20, "503b007b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 14, 20, "503b007c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 14, 20, "503b007d", 2, 255, -1, 0, 0, &_state3, 4 },
	{ 21, 21, "503b008a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 21, 21, "503b008b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 22, 25, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 25, 25, nullptr,    0,   0, 18, 0, 0, &_state5, 1 },
	{ 26, 26, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 27, -1, nullptr,    0,   0, 11, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY7[] = {
	{  4,  7, nullptr,    0,   0, 10, 0, 0, nullptr,  0 },
	{  8, 10, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 11, 13, "503b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 13, "503b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 14, 20, "503b007a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 14, 20, "503b007b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 14, 20, "503b007c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 14, 20, "503b007d", 2, 255, -1, 0, 0, &_state3, 4 },
	{ 21, 21, "503b008a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 21, 21, "503b008b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 22, 25, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY8[] = {
	{ 0,  3, "503_007",  3, 255, -1, 0, 0, nullptr,  0  },
	{ 4,  7, "503b002a", 2, 255, -1, 4, 3, &_state1, 1 },
	{ 4,  7, "503b002b", 2, 255, -1, 4, 3, &_state1, 2 },
	{ 4,  7, "503b002c", 2, 255, -1, 4, 3, &_state1, 3 },
	{ 8, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY9[] = {
	{ 0, -1, "503b003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY10[] = {
	{  0, 10, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 11, 14, "503b007a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 14, "503b007b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 11, 14, "503b007c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 11, 14, "503b007d", 2, 255, -1, 0, 0, &_state1, 4 },
	{ 15, 21, "503b010a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 15, 21, "503b010b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 15, 21, "503b010c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 22, 23, "500_003",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 24, 29, "503b011a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 24, 29, "503b011b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 30, 34, "503b012a", 2, 255, -1, 0, 0, &_state6, 1 },
	{ 30, 34, "503b012b", 2, 255, -1, 0, 0, &_state6, 2 },
	{ 35, 35, nullptr,    0,   0, -1, 0, 0, &_state5, 0 },
	{ 35, 35, nullptr,    0,   0, 18, 0, 0, &_state5, 1 },
	{ 36, 40, nullptr,    0,   0, 11, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY11[] = {
	{  0,  3, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  4, 11, nullptr,    0,   0, 10, 0, 0, nullptr, 0 },
	{ 12, 15, "503_006",  2, 255, -1, 0, 0, nullptr, 0 },
	{ 16, -1, "503b013a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 16, -1, "503b013b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 16, -1, "503b013c", 2, 255, -1, 0, 0, &_state1, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY12[] = {
	{ 0, -1, "503b014a", 2, 255, -1, 0, 3, &_state1, 1 },
	{ 0, -1, "503b014b", 2, 255, -1, 0, 3, &_state1, 2 },
	{ 0, -1, "503b014c", 2, 255, -1, 0, 3, &_state1, 3 },
	{ 0, -1, "503b014d", 2, 255, -1, 0, 3, &_state1, 4 },
	{ 0, -1, "503b014e", 2, 255, -1, 0, 3, &_state1, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY13[] = {
	{ 0, -1, "503_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY14[] = {
	{ 0, -1, "503_003", 2, 255, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY15[] = {
	{  0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  8, 10, "503_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 11, 15, "503_004", 3, 255, -1, 0, 0, nullptr, 0 },
	{ 16, 17, "503b015", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 18, -1, nullptr,   0,   0, 22, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY16[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY17[] = {
	{ 0, 0, nullptr,    0,   0, -1, 0,  6, &_state1, 1 },
	{ 0, 0, nullptr,    0,   0, -1, 0,  6, &_state1, 2 },
	{ 0, 0, nullptr,    0,   0, -1, 0,  6, &_state1, 3 },
	{ 0, 2, "503b017a", 2, 255, -1, 1, -1, &_state1, 4 },
	{ 0, 2, "503b017b", 2, 255, -1, 1, -1, &_state1, 5 },
	{ 0, 2, "503b017c", 2, 255, -1, 1, -1, &_state1, 6 },
	{ 0, 2, "503b017d", 2, 255, -1, 1, -1, &_state1, 7 },
	{ 0, 2, "503b017e", 2, 255, -1, 1, -1, &_state1, 8 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY18[] = {
	{ 0,  4, "503b018a", 2, 255, -1,    0, 0, &_state1, 1 },
	{ 0,  4, "503b018b", 2, 255, -1,    0, 0, &_state1, 2 },
	{ 0,  4, "503b018c", 2, 255, -1,    0, 0, &_state1, 3 },
	{ 0,  4, "503b018d", 2, 255, -1,    0, 0, &_state1, 4 },
	{ 5,  5, "500_001",  3, 255, -1,    0, 0, nullptr, 0 },
	{ 6, -1, nullptr,    2,   0, 12, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY19[] = {
	{  0,  1, "503_006",  2, 255, -1, 0, 0, nullptr,  0 },
	{  2,  8, "503_003",  2, 255, -1, 0, 0, nullptr,  0 },
	{  9, -1, "503b016a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  9, -1, "503b016b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  9, -1, "503b016c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ -1, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  0,  9, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 10, -1, nullptr,    0,   0,  8, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY20[] = {
	{  0, 23, "503_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 24, 35, "503_012", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 36, -1, "503_005", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY21[] = {
	{ 0, 6, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 2, 7, "503_004", 3, 255, -1, 2, 0, nullptr, 0 },
	{ 0, 1, nullptr,   0,   0, 22, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY22[] = {
	{  0, 10, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 11, -1, nullptr, 0, 0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY23[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

long Room503::_state1;
long Room503::_state2;
long Room503::_state3;
long Room503::_state4;
long Room503::_state5;
long Room503::_state6;

Room503::Room503() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
	_state5 = 0;
	_state6 = 0;
}

void Room503::init() {
}

void Room503::daemon() {
}

void Room503::pre_parser() {

}

void Room503::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
