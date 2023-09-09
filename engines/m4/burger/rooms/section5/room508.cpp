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

#include "m4/burger/rooms/section5/room508.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room508::SAID[][4] = {
	{ "WILBUR'S ROOM",      nullptr,   "500w001", nullptr   },
	{ "BORK",               "508w003", "500w002", "500w002" },
	{ "CHRISTMAS LIGHTS  ", "508w004", nullptr,   "508w007" },
	{ "CHRISTMAS LIGHTS   ","508w005", "508w006", "508w007" },
	{ "HOLE",               "508w008", "500w001", "508w009" },
	{ "BATHROOM",           "508w010", nullptr,   "508w011" },
	{ "CHIMNEY",            "508w012", "500w001", "508w014" },
	{ "CHIMNEY POTS",       "508w012", "500w001", "508w014" },
	{ "ROOF",               "508w015", "500w001", "508w016" },
	{ "FORCE FIELD",        "508w017", "500w001", "500w001" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room508::PLAY1[] = {
	{ 0, 4, "508_001", 1, 255, -1, 1, 0, nullptr, 0  },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY2[] = {
	{ 0,  4, "508_001", 1, 255, -1, 1024, 0, nullptr, 0 },
	{ 5, -1, nullptr,   0,   0,  5,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY3[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, 28, "508_002", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, -1, nullptr,   1,   0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY4[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, 27, "508_002", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 28, -1, nullptr,   0,   0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY7[] = {
	{ 0, -1, "508b001a", 2, 255, -1, 1025, 0, &_state1, 1 },
	{ 0, -1, "508b001b", 2, 255, -1, 1025, 0, &_state1, 2 },
	{ 0, -1, "508b001c", 2, 255, -1, 1025, 0, &_state1, 3 },
	{ 0, -1, "508b001d", 2, 255, -1, 1025, 0, &_state1, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY8[] = {
	{  0,  5, "508b003",  2, 255, -1,    0, 0, nullptr,  0 },
	{  6, 16, "500_005",  2, 255, -1,    0, 0, nullptr,  0 },
	{ 17, -1, "508b001a", 2, 255, -1, 1024, 0, &_state1, 1 },
	{ 17, -1, "508b001b", 2, 255, -1, 1024, 0, &_state1, 2 },
	{ 17, -1, "508b001c", 2, 255, -1, 1024, 0, &_state1, 3 },
	{ 17, -1, "508b001d", 2, 255, -1, 1024, 0, &_state1, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY9[] = {
	{  0,  5, nullptr,    0,   0, -1,    0, 0, nullptr,  0 },
	{  6, 12, "500_006",  2, 255, -1,    0, 0, nullptr,  0 },
	{ 13, 18, "508b005a", 2, 255, -1,    4, 6, &_state1, 1 },
	{ 13, 18, "508b005b", 2, 255, -1,    4, 6, &_state1, 2 },
	{ 19, -1, "508b001a", 2, 255, -1, 1024, 0, &_state2, 1 },
	{ 19, -1, "508b001b", 2, 255, -1, 1024, 0, &_state2, 2 },
	{ 19, -1, "508b001c", 2, 255, -1, 1024, 0, &_state2, 3 },
	{ 19, -1, "508b001d", 2, 255, -1, 1024, 0, &_state2, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY10[] = {
	{  0, 20, "508b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0, 20, "508b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  0, 20, "508b002c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 21, -1, nullptr,    0,   0,  1, 0, 0, &_state3, 1 },
	{ 21, -1, nullptr,    0,   0,  1, 0, 0, &_state3, 2 },
	{ 21, -1, nullptr,    0,   0,  2, 0, 0, &_state3, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY11[] = {
	{  0, 10, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 11, 17, "508b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 17, "508b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 11, 17, "508b006c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 11, 17, "508b006d", 2, 255, -1, 0, 0, &_state1, 4 },
	{ 18, 18, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 19, 28, "508b007a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 19, 28, "508b007b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 19, 28, "508b007c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 29, 42, "508b008a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 29, 42, "508b008b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 29, 42, "508b008c", 2, 255, -1, 0, 0, &_state4, 3 },
	{ 43, -1, "508b009",  2, 255, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY12[] = {
	{  0, 12, "500_003",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 13, -1, "508b010a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 13, -1, "508b010b", 2, 255, -1, 0, 0, &_state1, 2 },
	PLAY_BREAK_END
};

long Room508::_state1;
long Room508::_state2;
long Room508::_state3;
long Room508::_state4;


Room508::Room508() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
}

void Room508::init() {
}

void Room508::daemon() {
}

void Room508::pre_parser() {

}

void Room508::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
