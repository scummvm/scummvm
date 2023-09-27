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

#include "m4/burger/rooms/section1/room145.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room145::SAID[][4] = {
	{ "TOUR BUS",      "145w001", "145w002", "145w003" },
	{ "INSTRUMENTS",   "145w005", "145w006", "145w006" },
	{ "AMPLIFIER ",    "145w009", nullptr,   "145w012" },
	{ "VIPE",          "145w013", "145w004", "145w004" },
	{ "ROXY",          "145w016", "145w004", "145w004" },
	{ "DRUMZ",         "145w019", "145w004", "145w004" },
	{ "MAP",           "145w022", "145w002", "145w023" },
	{ "VERA'S DINER",  "145w024", "145w004", "145w004" },
	{ "VERA'S DINER ", "145w024", "145w004", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const WilburMatch Room145::MATCH[] = {
	{ nullptr, "TOUR BUS", 1, 0, 0, &_state1, 12 },
	{ "TALK", "ROXY", 10016, 0, 0, &Vars::_wilbur_should, 11 },
	{ "AMPLIFIER", "ROXY", 1, 0, 0, &_state1, 15 },
	{ nullptr, "ROXY", 1, 0, 0, &_state1, 17 },
	{ "TALK" , "DRUMZ", 10, 0, 0, 0, 0 }, 
	{ "LAXATIVE" , "DRUMZ", 10016, 0, 0, &Vars::_wilbur_should, 3 },
	{ nullptr, "DRUMZ", 1, 0, 0, &_state1, 16 },
	{ "TALK" , "VIPE", 10016, 0, 0, &Vars::_wilbur_should, 10 },
	{ "AMPLIFIER", "VIPE", 1, 0, 0, &_state1, 15 },
	{ nullptr, "VIPE", 1, 0, 0, &_state1, 16 },
	{ "AMPLIFIER" , "INSTRUMENTS", 1, 0, 0, &_state1, 13 },
	{ nullptr, "INSTRUMENTS", 1, 0, 0, &_state1, 14 },
	{ "TAKE", "AMPLIFIER ", 10016, &Flags::_flags[V067], 0, &Vars::_wilbur_should, 1 },
	{ "TAKE", "AMPLIFIER ", 10016, &Flags::_flags[V067], 1, &Vars::_wilbur_should, 8 },
	{ nullptr, "AMPLIFIER ", 1, 0, 0, &_state1, 14 },
	{ nullptr, "MAP", 1, 0, 0, &_state1, 18 },
	WILBUR_MATCH_END
};

const seriesPlayBreak Room145::PLAY1[] = {
	{ 0, 6, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 6, 6, nullptr,   0,   0, -1, 0,  6, nullptr, 0 },
	{ 6, 6, "145w010", 1, 255, -1, 0, -1, nullptr, 0 },
	{ 0, 6, nullptr,   0,   0, -1, 2,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY2[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, -1, "145w011", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY3[] = {
	{ 15, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY4[] = {
	{ 0, 3, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 4, 4, nullptr,   0,   0, -1, 0,  4, nullptr, 0 },
	{ 5, 9, "145w021", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 4, 4, nullptr,   0,   0, -1, 0,  4, nullptr, 0 },
	{ 0, 3, nullptr,   0,   0, -1, 2,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY7[] = {
	{ 0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2, -1, nullptr, 0, 0, -1, 0, 4, nullptr, 0 },
	{ 0,  1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY8[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY9[] = {
	{ 0, -1, "145_004", 2, 125, -1, 1, 0, &_state2, 0 },
	{ 0, -1, nullptr,   0,   0, -1, 0, 0, &_state2, 1 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY10[] = {
	{ 0, -1, nullptr, 0, 0, -1, 3, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY11[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY12[] = {
	{ 0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2,  4, nullptr, 0, 0, -1, 1, 2, nullptr, 0 },
	{ 5, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY13[] = {
	{  0,  5, nullptr,   0,   0,    -1, 0, 0, nullptr, 0 },
	{  6, 16, nullptr,   0,   0, 10016, 0, 0, nullptr, 0 },
	{ 17, 20, "145_002", 1, 255,    -1, 0, 0, nullptr, 0 },
	{ 21, 25, "145_005", 1, 255,    -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY14[] = {
	{ 26, -1, "145_001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY15[] = {
	{ 0, 3, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY16[] = {
	{ 0, 3, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY17[] = {
	{ 1, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY18[] = {
	{ 0, -1, "145r901", 2, 125, -1, 4, -1, &_state3, 1 },
	{ 0, -1, "145r902", 2, 125, -1, 4, -1, &_state3, 2 },
	{ 0, -1, "145r903", 2, 125, -1, 4, -1, &_state3, 3 },
	{ 0, -1, "145r904", 2, 125, -1, 4, -1, &_state3, 4 },
	{ 0, -1, "145r905", 2, 125, -1, 4, -1, &_state3, 5 },
	{ 0, -1, "145r906", 2, 125, -1, 4, -1, &_state3, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY19[] = {
	{ 1, -1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY20[] = {
	{ 1, -1, nullptr, 0, 0, -1, 0, 2, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY21[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY22[] = {
	{ 0, -1, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY23[] = {
	{ 0, -1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

long Room145::_state1;
long Room145::_state2;
long Room145::_state3;

Room145::Room145() : Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
}

void Room145::init() {

}

void Room145::daemon() {

}

void Room145::pre_parser() {

}

void Room145::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
