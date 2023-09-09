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

#include "m4/burger/rooms/section5/room506.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room506::SERIES1[] = {
	{   0, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  13, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  20, "506_006",  1, 255, -1, 0, nullptr,  0 },
	{  45, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  67, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  77, "506_003",  2, 255, -1, 0, nullptr,  0 },
	{  91, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 100, "506b005a", 1, 255, -1, 0, &_state1, 1 },
	{ 100, "506b005b", 1, 255, -1, 0, &_state1, 2 },
	{ 100, "506b005c", 1, 255, -1, 0, &_state1, 3 },
	{ 113, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 135, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 155, "506_001",  2, 255, -1, 0, nullptr,  0 },
	{ 165, nullptr,    0,   0, 11, 0, nullptr,  0 },
	STREAM_BREAK_END
};

const char *Room506::SAID[][4] = {
	{ "HALLWAY",        nullptr,   "500w001", nullptr   },
	{ "WINDOW",         "500w003", "500w001", nullptr   },
	{ "ROOF",           "500w003", "500w001", nullptr   },
	{ "BORK",           "506w004", "500w002", "500w002" },
	{ "TELEVISION",     "506w005", nullptr,   "506w007" },
	{ "FIRE",           "506w009", "506w010", "506w011" },
	{ "VIDEO GAME",     "506w012", "506w013", "506w015" },
	{ "BED",            "506w017", nullptr,   "506w018" },
	{ "MATTRESS",       "506w017", nullptr,   "506w018" },
	{ "READING LAMP",   "506w019", "506w020", "506w021" },
	{ "NIGHT TABLE",    "506w022", "500w005", "500w005" },
	{ "PILLOW",         "506w023", "506w024", "506w025" },
	{ "LAMP",           "506w026", "506w020", "506w021" },
	{ "BOOK",           "506w027", "506w028", "506w028" },
	{ "WARDROBE",       "506w029", nullptr,   nullptr   },
	{ "CLOTHES",        "506w030", "506w031", "506w032" },
	{ "DRESSER",        "506w033", nullptr,   nullptr   },
	{ "MODEL ROCKET ",  "506w034", "500w005", "500w005" },
	{ "MODEL ROCKET",   "506w035", "506w036", "500w005" },
	{ "MODEL ROCKET  ", "506w037", "500w005", "500w005" },
	{ "POSTER ",        "506w038", "500w005", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room506::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY3[] = {
	{  0, 25, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 26, 40, nullptr,   0,   0,  5, 0,  0, nullptr, 0 },
	{ 41, 46, "500w067", 1, 255, -1, 4, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY4[] = {
	{  9, 22, "506_003",  2,  50, -1, 0, 0, nullptr,  0 },
	{ 23, 28, "506b003a", 2, 255, -1, 4, 2, &_state1, 1 },
	{ 23, 28, "506b003b", 2, 255, -1, 4, 2, &_state1, 2 },
	{ 29, 37, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY5[] = {
	{ 38, 38, "506_003",  2, 120, -1, 0, 12, nullptr,  0 },
	{ 39, 73, "506b004a", 2, 255, -1, 0,  0, &_state1, 1 },
	{ 39, 73, "506b004b", 2, 255, -1, 0,  0, &_state1, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY6[] = {
	{  0, 12, "506b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0, 12, "506b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 13, 13, nullptr,    0,   0,  9, 0, 0, &_state2, 3 },
	{ 13, 30, nullptr,    0,   0,  6, 0, 0, nullptr,  0 },
	{ 31, -1, nullptr,    0,   0,  2, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};


long Room506::_state1;
long Room506::_state2;

Room506::Room506() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room506::init() {
}

void Room506::daemon() {
}

void Room506::pre_parser() {

}

void Room506::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
