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

#include "m4/burger/rooms/section5/room507.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room507::SAID[][4] = {
	{ "HALLWAY",            nullptr,   "500w001", nullptr   },
	{ "BORK",               "507w001", "500w002", "500w002" },
	{ "RUBBER DUCKY ",      "507w004", nullptr,   "507w006" },
	{ "RUBBER DUCKY  ",     "507w004", "507w005", "507w006" },
	{ "TOILET",             "507w007", "500w001", nullptr   },
	{ "FLUSH CHAIN",        "507w007", "500w001", nullptr   },
	{ "TUB",                "507w001", "500w001", "500w002" },
	{ "WINDOW",             "500w003", "500w001", "500w004" },
	{ "TOILET SEAT",        "507w012", "500w005", "500w005" },
	{ "SHOWER CURTAIN",     nullptr,   "500w005", nullptr   },
	{ "SHOWERHEAD",         "507w013", "507w014", nullptr   },
	{ "CLOSET",             "507w015", "500w001", "500w001" },
	{ "TOILET PAPER",       "507w016", "500w005", "500w005" },
	{ "TOWELS",             "507w017", "507w018", "507w018" },
	{ "DRAIN CLEANER",      "507w019", "507w020", "507w020" },
	{ "MIRROR",             "507w021", nullptr,   "507w022" },
	{ "SINK",               "507w023", nullptr,   nullptr   },
	{ "BOARD",              "507w024", "500w005", nullptr   },
	{ "LIVER SPOT CREAM",   "507w025", "507w026", "507w026" },
	{ "FACIAL HAIR BLEACH", "507w027", "507w028", "507w029" },
	{ "WRINKLE CREAM",      "507w030", "507w031", "507w031" },
	{ "FACE CREAM",         "507w032", "507w020", "507w033" },
	{ "WIRES",              "507w034", "507w035", "507w035" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room507::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY2[] = {
	{ 0,  4, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 5, -1, nullptr, 0, 0, 11, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY3[] = {
	{ 0, 5, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 5, "500_004", 1, 255, -1, 0, 3, nullptr, 0 },
	{ 0, 3, nullptr,   0,   0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY4[] = {
	{ 11, 16, nullptr,   0,   0,    -1, 0, 0, nullptr, 0 },
	{ 17, 19, "507_006", 1, 255,    11, 0, 0, nullptr, 0 },
	{ 20, 23, nullptr,   0,   0, 10016, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY5[] = {
	{ 0, 10, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY6[] = {
	{ 0, 10, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY7[] = {
	{ 0, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0,  9, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY8[] = {
	{  0, 12, nullptr,   0,   0, -1,    0, 0, nullptr,  0 },
	{ 13, 14, "507_004", 1, 255, -1,    0, 0, nullptr,  0 },
	{ 15, 15, nullptr,   0,   0, -1,    0, 8, nullptr,  0 },
	{ 16, 27, nullptr,   1,   0, -1, 2048, 0, nullptr,  0 },
	{ 28, -1, nullptr,   0,   0, 11,    0, 0, nullptr, -1 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY9[] = {
	{  0,  6, "507b001a", 3, 255, -1, 0, 0, &_state1, 1 },
	{  0,  6, "507b001b", 3, 255, -1, 0, 0, &_state1, 2 },
	{  0,  6, "507b001c", 3, 255, -1, 0, 0, &_state1, 3 },
	{  0,  6, "507b001d", 3, 255, -1, 0, 0, &_state1, 4 },
	{  7, 11, "507_002a", 2, 255, -1, 0, 0, &_state2, 1 },
	{  7, 11, "507_002b", 2, 255, -1, 0, 0, &_state2, 2 },
	{  7, 11, "507_002c", 2, 255, -1, 0, 0, &_state2, 3 },
	{  7, 11, "507_002d", 2, 255, -1, 0, 0, &_state2, 4 },
	{  7, 11, "507_002e", 2, 255, -1, 0, 0, &_state2, 5 },
	{  7, 11, "507_002f", 2, 255, -1, 0, 0, &_state2, 6 },
	{ 12, 21, "507_002a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 12, 21, "507_002b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 12, 21, "507_002c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 12, 21, "507_002d", 2, 255, -1, 0, 0, &_state3, 4 },
	{ 12, 21, "507_002e", 2, 255, -1, 0, 0, &_state3, 5 },
	{ 12, 21, "507_002f", 2, 255, -1, 0, 0, &_state3, 6 },
	{ 22, -1, "507_002a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 22, -1, "507_002b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 22, -1, "507_002c", 2, 255, -1, 0, 0, &_state4, 3 },
	{ 22, -1, "507_002d", 2, 255, -1, 0, 0, &_state4, 4 },
	{ 22, -1, "507_002e", 2, 255, -1, 0, 0, &_state4, 5 },
	{ 22, -1, "507_002f", 2, 255, -1, 0, 0, &_state4, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY10[] = {
	{ 0, -1, "507_001", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY11[] = {
	{  0,  1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  2,  9, "507b003a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  2,  9, "507b003b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  2,  9, "507b003c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 10, 15, "507b004a", 2, 255, -1, 4, 2, &_state2, 1 },
	{ 10, 15, "507b004b", 2, 255, -1, 4, 2, &_state2, 2 },
	{ 10, 15, "507b004c", 2, 255, -1, 4, 2, &_state2, 3 },
	{ 16, 20, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 21, 21, nullptr,    0,   0, -1, 0, 8, nullptr,  0 },
	{ 22, -1, "507b005",  2, 255, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY12[] = {
	{  0, 11, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 12, 15, "507b006a", 2, 255, -1, 4, 2, &_state1, 1 },
	{ 12, 15, "507b006b", 2, 255, -1, 4, 2, &_state1, 2 },
	{ 16, 16, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 17, 18, "500_004",  2, 255, -1, 0, 3, nullptr,  0 },
	{ 19, 28, "507b007a", 2, 255, -1, 4, 2, &_state2, 1 },
	{ 19, 28, "507b007b", 2, 255, -1, 4, 2, &_state2, 2 },
	{ 29, 32, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 33, -1, "507b008a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 33, -1, "507b008b", 2, 255, -1, 0, 0, &_state3, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY13[] = {
	{  0,  3, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  4,  8, "507b003a", 2, 255, -1, 4, 0, &_state1, 1 },
	{  4,  8, "507b003b", 2, 255, -1, 4, 0, &_state1, 2 },
	{  4,  8, "507b003c", 2, 255, -1, 4, 0, &_state1, 3 },
	{ 20, 37, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 38, 46, "507b010a", 2, 255, -1, 1, 0, &_state2, 1 },
	{ 38, 46, "507b010b", 2, 255, -1, 1, 0, &_state2, 2 },
	{ 47, 59, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 60, 78, "500_014",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 79, -1, "507_007",  2, 255, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY14[] = {
	{  0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  8, 13, "507b011", 2, 255, -1, 4, 0, nullptr, 0 },
	{ 14, 29, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 30, -1, "507_005", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY15[] = {
	{ 0, -1, "507b013", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY16[] = {
	{ 0,  6, "507b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 0,  6, "507b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 0,  6, "507b002c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 7,  7, nullptr,    0,   0,  8, 0, 0, &_state5, 3 },
	{ 7, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY17[] = {
	{ 0,  3, "507_003", 1, 255, -1, 1, 5, nullptr, 0 },
	{ 4, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room507::PLAY18[] = {
	{ 6, 6, nullptr, 0, 0, -1, 0, 5, nullptr, 0 },
	{ 5, 5, nullptr, 0, 0, -1, 0, 5, nullptr, 0 },
	{ 4, 4, nullptr, 0, 0, -1, 0, 5, nullptr, 0 },
	PLAY_BREAK_END
};

long Room507::_state1;
long Room507::_state2;
long Room507::_state3;
long Room507::_state4;
long Room507::_state5;


Room507::Room507() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
	_state5 = 0;
}

void Room507::init() {
}

void Room507::daemon() {
}

void Room507::pre_parser() {

}

void Room507::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
