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

#include "m4/burger/rooms/section5/room505.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room505::SAID[][4] = {
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

const Section5Room::BorkPoint Room505::BORK[] = {
	{ -1, -1 }, { 257, 255 }, { 266, 257 }, { 271, 250 }, { 292, 246 },
	{ 317, 251 }, { 323, 256 }, { 338, 237 }, { 370, 227 }, { 348, 187 },
	{ 332, 165 }, { 308, 161 }, { 282, 161 }, { 303, 175 }, { 325, 179 },
	{ 374, 174 }, { 350, 172 }, { 327, 168 }, { 302, 167 }, { 282, 161 },
	{ 303, 175 }, { 325, 179 }, { 374, 174 }, { 354, 175 }, { 330, 183 },
	{ 317, 204 }, { 305, 217 }, { 284, 210 }, { 270, 227 }, { 255, 236 },
	{ -1, -1 }, { -1, -1 }
};

const seriesPlayBreak Room505::PLAY1[] = {
	{ 0,  3, "505_003a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 4, -1, "505_003b", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room505::PLAY2[] = {
	{ 0,  2, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{ 3, -1, "505_003a", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room505::init() {
}

void Room505::daemon() {
}

void Room505::pre_parser() {

}

void Room505::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
