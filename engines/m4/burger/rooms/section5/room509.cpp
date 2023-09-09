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

#include "m4/burger/rooms/section5/room509.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room509::SAID[][4] = {
	{ "HALLWAY",            nullptr, "500w001",   nullptr   },
	{ "CHRISTMAS LIGHTS  ", "509w002", nullptr,   nullptr   },
	{ "PHONE CORD",         "509w005", "509w006", "509w007" },
	{ "HOLE",               "509w008", "500w001", "509w009" },
	{ "ROOF BORK",          nullptr,   nullptr,   nullptr   },
	{ "BED",                "509w010", nullptr,   "509w011" },
	{ "MATTRESS",           "509w010", nullptr,   "509w011" },
	{ "PILLOW",             "509w012", "509w013", "509w013" },
	{ "LAMP",               "509w014", "509w015", "509w014" },
	{ "WINDOW",             "500w003", nullptr,   "500w004" },
	{ "CHAIR",              "509w016", nullptr,   "509w017" },
	{ "TELEPHONE",          "509w018", "509w019", "509w020" },
	{ "BOOK",               "509w021", "509w022", "509w023" },
	{ "PICTURE",            "509w024", "500w005", nullptr   },
	{ "PICTURE ",           "509w025", "500w005", "509w026" },
	{ "PICTURE  ",          "509w027", "500w005", "509w026" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room509::PLAY1[] = {
	{  0, 14, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 18, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY3[] = {
	{  0, -1, nullptr,   0,   0, -1, 0,  0, nullptr,  0 },
	{  0, 15, nullptr,   0,   0, -1, 0,  0, nullptr,  0 },
	{ 16, -1, nullptr,   0,   0,  3, 0,  0, nullptr,  0 },
	{ 16, -1, "500w086", 1, 255, -1, 4, -1, &_state1, 1 },
	{ 16, -1, nullptr,   0,   0, -1, 4,  2, &_state1, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY4[] = {
	{  0, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  0, 15, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 16, -1, "509_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY7[] = {
	{ 0, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, "509_002", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY8[] = {
	{ 0,  4, "508b003", 2, 255, -1, 0, 0, nullptr,  0 },
	{ 5, -1, "500_005", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 5, -1, "500_006", 2, 255, -1, 0, 0, &_state2, 2 },
	PLAY_BREAK_END
};

long Room509::_state1;
long Room509::_state2;


Room509::Room509() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room509::init() {
}

void Room509::daemon() {
}

void Room509::pre_parser() {

}

void Room509::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
