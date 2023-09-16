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

#include "m4/burger/rooms/section4/room407.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room407::SAID[][4] = {
	{ "TOUR BUS",     "407w001", "407w002", "407w003" },
	{ "ROXY",         nullptr,   "400w001", "400w001" },
	{ "DRUMZ",        "407w007", "400w001", "400w001" },
	{ "INSTRUMENTS",  "407w009", "407w010", "407w010" },
	{ "FORCE FIELD",  "400w005", nullptr,   "400w001" },
	{ "FORCE FIELD ", "400w005", nullptr,   "400w001" }
};

const WilburMatch Room407::MATCH[] = {
	{ "TALK", "DRUMZ",           11, 0, 0, nullptr, 0 },
	{ "AMPLIFIER", "DRUMZ",       4, 0, 0, &_state1, 4 },
	{ nullptr, "DRUMZ",           4, 0, 0, &_state1, 7 },
	{ "LOOK AT", "ROXY",      10016, 0, 0, &Vars::_wilbur_should, 1 },
	{ "TALK", "ROXY",         10016, 0, 0, &Vars::_wilbur_should, 2 },
	{ "AMPLIFIER", "ROXY",        4, 0, 0, &_state1, 4 },
	{ "DOG COLLAR", "ROXY",       4, 0, 0, &_state1, 5 },
	{ nullptr, "ROXY",            4, 0, 0, &_state1, 6 },
	{ "AMPLIFIER", "INSTRUMENTS", 4, 0, 0, &_state1, 8 },
	{ nullptr, "INSTRUMENTS",     4, 0, 0, &_state1, 9 },
	{ nullptr, "TOUR BUS",        4, 0, 0, &_state1, 3 },
	{ nullptr, nullptr, -1, 0, 0, nullptr, 0 }
};

const seriesPlayBreak Room407::PLAY1[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY2[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY3[] = {
	{ 0, 1, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 2, -1, 0, 0, 0, -1, 0, 4, 0, 0 },
	{ 0, 1, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY4[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY5[] = {
	{ 0, -1, "145_004", 2, 255, -1, 1, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY6[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY7[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY8[] = {
	{ 0, -1, "407r901", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY9[] = {
	{ 0, -1, "407r903", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY10[] = {
	{ 0, -1, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY11[] = {
	{ 0, 2, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 3, -1, "407r904x", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY12[] = {
	{ 0, 6, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY13[] = {
	{ 7, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY14[] = {
	{ 0, -1, "407r904y", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY15[] = {
	{ 0, 3, "407v901", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY16[] = {
	{ 5, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY17[] = {
	{ 1, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY18[] = {
	{ 0, 1, "999_003", 1, 255, -1, 0, 0, 0, 0 },
	{ 2, 9, 0, 0, 0, 7, 0, 0, 0, 0 },
	{ 10, 11, "999_003", 1, 255, -1, 0, 0, 0, 0 },
	{ 12, -1, 0, 0, 0, 9, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY19[] = {
	{ 0, 25, 0, 1, 0, -1, 2048, 0, 0, 0 },
	{ 26, 42, "500_030", 1, 255, -1, 0, 0, 0, 0 },
	{ 43, -1, "500_030", 1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

long Room407::_state1;


Room407::Room407() : Room() {
	_state1 = 0;
}

void Room407::init() {
}

void Room407::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
