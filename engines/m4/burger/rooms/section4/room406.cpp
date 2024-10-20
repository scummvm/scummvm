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

#include "m4/burger/rooms/section4/room406.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"
#include "m4/burger/burger.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_MAYOR_ANIMATION = 11,
	kCHANGE_DOG_ANIMATION = 12
};

const char *Room406::SAID[][4] = {
	{ "PEGLEG",         "406w003", "406w005", "406w005" },
	{ "PEGLEG ",        "406w003", "406w005", "406w005" },
	{ "PEGLEG  ",       "406w004", "406w005", "406w005" },
	{ "TIRE",           "406w007", "406w008", "406w009" },
	{ "TIRE ",          "406w028", nullptr,   nullptr   },
	{ "DOG COLLAR ",    "406w010", nullptr,   "406w011" },
	{ "DOG COLLAR  ",   "406w010", nullptr,   "406w011" },
	{ "DOG COLLAR   ",  "406w010", nullptr,   "406w011" },
	{ "DOG COLLAR    ", "406w012", nullptr,   "406w011" },
	{ "GATE",           "406w013", "400w001", nullptr   },
	{ "WINCH",          "406w016", "400w001", nullptr   },
	{ "HOOK ",          nullptr,   nullptr,   "400w001" },
	{ "HOOK  ",         nullptr,   nullptr,   "400w001" },
	{ "HOOK   ",        nullptr,   nullptr,   "400w001" },
	{ "HOOK    ",       nullptr,   nullptr,   "400w001" },
	{ "BARRED WINDOW ", "406w024", "400w001", nullptr   },
	{ "JAIL CELL",      nullptr,   "400w001", "400w001" },
	{ "TOW TRUCK",      "406w025", "400w001", nullptr   },
	{ "FENCE",          nullptr,   "400w001", nullptr   },
	{ "RAZOR WIRE",     nullptr,   "406w033", "406w033" },
	{ "RAZOR WIRE ",    nullptr,   "406w033", "406w033" },
	{ "SIGN",           "406w034", nullptr,   nullptr   },
	{ "RUBBLE",         "406w035", nullptr,   nullptr   },
	{ "GARBAGE CANS",   "406w036", "406w037", nullptr   },
	{ "FORCE FIELD",    "400w005", nullptr,   "400w001" },
	{ "FORCE FIELD ",   "400w005", nullptr,   "400w001" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const WilburMatch Room406::MATCH[] = {
	{ "FISH", "PEGLEG", 8, 0, 0, &_state4, 48 },
	{ "FISH", "PEGLEG ", 8, 0, 0, &_state4, 48 },
	{ "FISH", "TIRE", 10016, 0, 0, &Vars::_wilbur_should, 2 },
	{ "GEAR", "GATE", 8, &Flags::_flags[V172], 10025, &_state4, 25 },
	{ "GEAR", "GATE", 8, 0, 0, &_state4, 26 },
	{ "JAWZ O' LIFE", "GATE", 10016, &Flags::_flags[V172], 10025, &Vars::_wilbur_should, 16 },
	{ "JAWZ O' LIFE", "GATE", 8, 0, 0, &_state4, 27 },
	{ "JAWZ O' LIFE", "LOCK", 10016, &Flags::_flags[V172], 10025, &Vars::_wilbur_should, 16 },
	{ "JAWZ O' LIFE", "LOCK", 8, 0, 0, &_state4, 27 },
	{ "JAWZ O' LIFE", "CHAIN", 10016, &Flags::_flags[V172], 10025, &Vars::_wilbur_should, 16 },
	{ "JAWZ O' LIFE", "CHAIN", 8, 0, 0, &_state4, 27 },
	{ "TAKE", "DOG COLLAR   ", 10016, 0, 0, &Vars::_wilbur_should, 20 },
	{ "TAKE", "DOG COLLAR    ", 10016, 0, 0, &Vars::_wilbur_should, 18 },
	{ "GEAR", "WINCH", 8, &Flags::_flags[V175], 0, &_state4, 36 },
	{ "GEAR", "WINCH", 10016, &Flags::_flags[V174], 4000, &Vars::_wilbur_should, 3 },
	{ "GEAR", "WINCH", 10016, &Flags::_flags[V174], 4001, &Vars::_wilbur_should, 4 },
	{ "GEAR", "WINCH", 8, &Flags::_flags[V174], 4002, &_state4, 34 },
	{ "GEAR", "WINCH", 8, &Flags::_flags[V174], 4004, &_state4, 35 },
	{ "GEAR", "LEVER", 8, &Flags::_flags[V175], 0, &_state4, 36 },
	{ "GEAR", "LEVER", 10016, &Flags::_flags[V174], 4000, &Vars::_wilbur_should, 3 },
	{ "GEAR", "LEVER ", 10016, &Flags::_flags[V174], 4001, &Vars::_wilbur_should, 4 },
	{ "GEAR", "LEVER ", 8, &Flags::_flags[V174], 4002, &_state4, 34 },
	{ "GEAR", "LEVER", 8, &Flags::_flags[V174], 4004, &_state4, 35 },
	{ "LOOK AT", "HOOK ", 8, 0, 0, &_state4, 29 },
	{ "LOOK AT", "HOOK  ", 8, 0, 0, &_state4, 29 },
	{ "LOOK AT", "HOOK   ", 8, 0, 0, &_state4, 30 },
	{ "LOOK AT", "HOOK    ", 8, 0, 0, &_state4, 29 },
	{ "TAKE", "HOOK ", 8, &Flags::_flags[V175], 0, &_state4, 36 },
	{ "TAKE", "HOOK ", 8, 0, 0, &_state4, 31 },
	{ "TAKE", "HOOK  ", 10016, 0, 0, &Vars::_wilbur_should, 6 },
	{ "TAKE", "HOOK   ", 8, 0, 0, &_state4, 32 },
	{ "LOOK AT", "JAIL CELL", 8, 0, 0, &_state4, 33 },
	{ "GEAR", "TOW TRUCK", 8, &Flags::_flags[V175], 0, &_state4, 36 },
	{ "GEAR", "TOW TRUCK", 8, &Flags::_flags[V175], 1, &_state4, 39 },
	{ "TAKE", "TIRE ", 8, &Flags::_flags[V175], 0, &_state4, 36 },
	{ "TAKE", "TIRE ", 8, &Flags::_flags[V175], 1, &_state4, 37 },
	{ "GEAR", "TIRE ", 8, &Flags::_flags[V175], 0, &_state4, 36 },
	{ "GEAR", "TIRE ", 8, &Flags::_flags[V175], 1, &_state4, 38 },
	{ "LOOK AT", "FENCE", 8, &Flags::_flags[V175], 0, &_state4, 40 },
	{ "LOOK AT", "FENCE", 8, &Flags::_flags[V175], 1, &_state4, 41 },
	{ "GEAR", "FENCE", 8, &Flags::_flags[V175], 0, &_state4, 42 },
	{ "GEAR", "FENCE", 8, &Flags::_flags[V175], 1, &_state4, 24 },
	{ "LOOK AT", "RAZOR WIRE", 8, &Flags::_flags[V175], 0, &_state4, 43 },
	{ "LOOK AT", "RAZOR WIRE", 8, &Flags::_flags[V175], 1, &_state4, 44 },
	{ "LOOK AT", "RAZOR WIRE ", 8, &Flags::_flags[V175], 0, &_state4, 43 },
	{ "LOOK AT", "RAZOR WIRE ", 8, &Flags::_flags[V175], 1, &_state4, 44 },
	{ nullptr, nullptr, -1, nullptr, 0, nullptr, 0 }
};

const seriesStreamBreak Room406::SERIES1[] = {
	{  6, "406_103", 1, 255, -1, 0, nullptr, 0 },
	{  9, "406_106", 1, 255, -1, 0, nullptr, 0 },
	{  33, nullptr,  0,   0, 16, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room406::PLAY1[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, 32, "406_112", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 33, -1, "406_110", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY2[] = {
	{  0,  6, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  7,  9, "406_102", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr,   0,   0, 15, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY3[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY4[] = {
	{ 0, 1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 2, 9, "406_103", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY5[] = {
	{  0,  4, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  5,  9, "406_103", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, -1, "406_107", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY6[] = {
	{ 0, 9, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY7[] = {
	{ 9, 7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, "406_108", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY8[] = {
	{ 10, 41, "406_105", 1, 255, -1, 1024, 0, nullptr, 0 },
	{ 42, -1, "406_109", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY9[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY10[] = {
	{ 0,  4, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 5, -1, "406_107", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY11[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY12[] = {
	{ 0,  1, "406_115", 1, 55, -1, 0, 0, nullptr, 0 },
	{ 2,  5, "406_115", 1, 55, -1, 0, 0, nullptr, 0 },
	{ 6,  8, "406_115", 1, 55, -1, 0, 0, nullptr, 0 },
	{ 9, -1, "406_115", 1, 55, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY13[] = {
	{ 0, -1, "406g003", 1, 255, -1, 4, -1, &_state1, 79 },
	{ 0, -1, "406g004", 1, 255, -1, 4, -1, &_state1, 80 },
	{ 0, -1, "406g005", 1, 255, -1, 4, -1, &_state1, 81 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY14[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 8,  4, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2,  0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY15[] = {
	{ 0, 19, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY16[] = {
	{ 20, 20, "406_102", 2, 255, -1,  0,  0, nullptr, 0 },
	{ 21, 23, "406g006", 1, 255, -1,  0,  0, nullptr, 0 },
	{ 24, 24, nullptr,   0,   0, 15,  0,  0, nullptr, 0 },
	{ 25, 25, nullptr,   0,   0, 17,  0,  0, nullptr, 0 },
	{ 26, 26, "406_114", 2, 255, -1,  0,  0, nullptr, 0 },
	{ 27, 31, "406g007", 1, 255, -1,  0,  0, nullptr, 0 },
	{ 32, -1, "406g008", 1, 255, 13, 32, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY17[] = {
	{ 20, 20, "406_102", 2, 255, -1,  0,  0, nullptr, 0 },
	{ 21, 23, "406g006", 1, 255, -1,  0,  0, nullptr, 0 },
	{ 24, 24, nullptr,   0,   0, 15,  0,  0, nullptr, 0 },
	{ 25, 25, "406_114", 2, 255, -1,  0,  0, nullptr, 0 },
	{ 26, 31, "406g007", 1, 255, -1,  0,  0, nullptr, 0 },
	{ 32, -1, "406g008", 1, 255, 13, 32, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY18[] = {
	{ 0, -1, "999_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY19[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY20[] = {
	{ 0, -1, "406d003a", 1, 255, -1, 0, 0, &_state2, 1 },
	{ 0, -1, "406d003b", 1, 255, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "406d003c", 1, 255, -1, 0, 0, &_state2, 3 },
	{ 0, -1, "406d003d", 1, 255, -1, 0, 0, &_state2, 4 },
	{ 0, -1, "406d003e", 1, 255, -1, 0, 0, &_state2, 5 },
	{ 0, -1, "406d003f", 1, 255, -1, 0, 0, &_state2, 6 },
	{ 0, -1, "406d003g", 1, 255, -1, 0, 0, &_state2, 7 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY21[] = {
	{ 13, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY22[] = {
	{ 0, -1, "406d001a", 2, 255, -1, 0, -1, &_state2, 1 },
	{ 0, -1, "406d001b", 2, 255, -1, 0, -1, &_state2, 2 },
	{ 0, -1, "406d001c", 2, 255, -1, 0, -1, &_state2, 3 },
	{ 0, -1, "406d001d", 2, 255, -1, 0, -1, &_state2, 4 },
	{ 0, -1, "406d001e", 2, 255, -1, 0, -1, &_state2, 5 },
	{ 0, -1, "406d001f", 2, 255, -1, 0, -1, &_state2, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY23[] = {
	{ 0, -1, "406d002a", 2, 255, -1, 0, -1, &_state2, 1 },
	{ 0, -1, "406d002b", 2, 255, -1, 0, -1, &_state2, 2 },
	{ 0, -1, "406d002c", 2, 255, -1, 0, -1, &_state2, 3 },
	{ 0, -1, "406d002d", 2, 255, -1, 0, -1, &_state2, 4 },
	{ 0, -1, "406d002e", 2, 255, -1, 0, -1, &_state2, 5 },
	{ 0, -1, "406d002f", 2, 255, -1, 0, -1, &_state2, 6 },
	{ 0, -1, "406d002g", 2, 255, -1, 0, -1, &_state2, 7 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY24[] = {
	{ 0, 4, "406d006a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 0, 4, "406d006b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 0, 4, "406d006c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 5, -1, "406_005", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY25[] = {
	{ 0,  4, "106_005",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 5, -1, "406d007a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 5, -1, "406d007b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 5, -1, "406d007c", 2, 255, -1, 0, 0, &_state2, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY26[] = {
	{ 0, -1, "406d005",  2, 255, -1, 0, 0, &_state2, 1 },
	{ 0, -1, "406d005b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "406d005c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 0, -1, "406d005d", 2, 255, -1, 0, 0, &_state2, 4 },
	{ 0, -1, "406d005e", 2, 255, -1, 0, 0, &_state2, 5 },
	{ 0, -1, "406d005f", 2, 255, -1, 0, 0, &_state2, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY27[] = {
	{ 0, -1, "406d005",  2, 255, -1, 0, 0, &_state2, 1 },
	{ 0, -1, "406d005b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "406d005c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 0, -1, "406d005d", 2, 255, -1, 0, 0, &_state2, 4 },
	{ 0, -1, "406d005e", 2, 255, -1, 0, 0, &_state2, 5 },
	{ 0, -1, "406d005f", 2, 255, -1, 0, 0, &_state2, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY28[] = {
	{ 0, -1, "406d005",  2, 255, -1, 0, 0, &_state2, 1 },
	{ 0, -1, "406d005b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "406d005c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 0, -1, "406d005d", 2, 255, -1, 0, 0, &_state2, 4 },
	{ 0, -1, "406d005e", 2, 255, -1, 0, 0, &_state2, 5 },
	{ 0, -1, "406d005f", 2, 255, -1, 0, 0, &_state2, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY29[] = {
	{  0, 20, "406d008a", 2, 255, -1, 0, 0, &_state2, 1 },
	{  0, 20, "406d008b", 2, 255, -1, 0, 0, &_state2, 2 },
	{  0, 20, "406d008c", 2, 255, -1, 0, 0, &_state2, 3 },
	{  0, 20, "406d008d", 2, 255, -1, 0, 0, &_state2, 4 },
	{ 21, -1, "406d009a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 21, -1, "406d009b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 21, -1, "406d009c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 21, -1, "406d009d", 2, 255, -1, 0, 0, &_state3, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY30[] = {
	{ 0, 1, nullptr,    2, 255, -1, 0, 10, &_state2, 0 },
	{ 0, 1, "406d011a", 2, 255, -1, 0, -1, &_state2, 1 },
	{ 0, 1, "406d011b", 2, 255, -1, 0, -1, &_state2, 2 },
	{ 0, 1, "406d011c", 2, 255, -1, 0, -1, &_state2, 3 },
	{ 0, 1, "406d011d", 2, 255, -1, 0, -1, &_state2, 4 },
	{ 0, 1, "406d011e", 2, 255, -1, 0, -1, &_state2, 5 },
	{ 0, 1, "406d011f", 2, 255, -1, 0, -1, &_state2, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY31[] = {
	{ 0, 3, "406d010a", 2, 175, -1, 0, 0, &_state2, 1 },
	{ 0, 3, "406d010b", 2, 175, -1, 0, 0, &_state2, 2 },
	{ 0, 3, "406d010c", 2, 175, -1, 0, 0, &_state2, 3 },
	{ 0, 3, "406d010d", 2, 175, -1, 0, 0, &_state2, 4 },
	{ 0, 3, "406d010e", 2, 175, -1, 0, 0, &_state2, 5 },
	{ 4, 4, nullptr,    0,   0, -1, 0, 4, nullptr,  0 },
	{ 0, 3, nullptr,    0,   0, -1, 2, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY32[] = {
	{ 0, 5, "406d010a", 2, 175, -1, 0, 0, &_state2, 1 },
	{ 0, 5, "406d010b", 2, 175, -1, 0, 0, &_state2, 2 },
	{ 0, 5, "406d010c", 2, 175, -1, 0, 0, &_state2, 3 },
	{ 0, 5, "406d010d", 2, 175, -1, 0, 0, &_state2, 4 },
	{ 0, 5, "406d010e", 2, 175, -1, 0, 0, &_state2, 5 },
	{ 6, 6, nullptr,    0,   0, -1, 0, 4, nullptr,  0 },
	{ 0, 5, nullptr,    0,   0, -1, 2, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY33[] = {
	{ 0, -1, "406d010a", 2, 175, -1, 0, 0, &_state2, 1 },
	{ 0, -1, "406d010b", 2, 175, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "406d010c", 2, 175, -1, 0, 0, &_state2, 3 },
	{ 0, -1, "406d010d", 2, 175, -1, 0, 0, &_state2, 4 },
	{ 0, -1, "406d010e", 2, 175, -1, 0, 0, &_state2, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY34[] = {
	{ 0, 3, "406d010a", 2, 175, -1, 0, 0, &_state2, 1 },
	{ 0, 3, "406d010b", 2, 175, -1, 0, 0, &_state2, 2 },
	{ 0, 3, "406d010c", 2, 175, -1, 0, 0, &_state2, 3 },
	{ 0, 3, "406d010d", 2, 175, -1, 0, 0, &_state2, 4 },
	{ 0, 3, "406d010e", 2, 175, -1, 0, 0, &_state2, 5 },
	{ 4, 4, nullptr,    0,   0, -1, 0, 4, nullptr,  0 },
	{ 2, 3, nullptr,    0,   0, -1, 2, 0, nullptr,  0 },
	{ 4, 4, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 5, 5, nullptr,    0,   0, -1, 0, 4, nullptr,  0 },
	{ 0, 5, nullptr,    0,   0, -1, 2, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY35[] = {
	{  0, 14, "406d010a", 2, 175, -1, 0, 0, &_state2, 1 },
	{  0, 14, "406d010b", 2, 175, -1, 0, 0, &_state2, 2 },
	{  0, 14, "406d010c", 2, 175, -1, 0, 0, &_state2, 3 },
	{  0, 14, "406d010d", 2, 175, -1, 0, 0, &_state2, 4 },
	{  0, 14, "406d010e", 2, 175, -1, 0, 0, &_state2, 5 },
	{ 15, 16, nullptr,    0,   0, -1, 1, 3, nullptr,  0 },
	{ 15, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room406::PLAY36[] = {
	{ 0,  2, "406d010a", 2, 175, -1, 0, 0, &_state2, 1 },
	{ 0,  2, "406d010b", 2, 175, -1, 0, 0, &_state2, 2 },
	{ 0,  2, "406d010c", 2, 175, -1, 0, 0, &_state2, 3 },
	{ 0,  2, "406d010d", 2, 175, -1, 0, 0, &_state2, 4 },
	{ 0,  2, "406d010e", 2, 175, -1, 0, 0, &_state2, 5 },
	{ 1,  0, nullptr,    0,   0, -1, 1, 0, nullptr,  0 },
	{ 2,  6, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 5,  4, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 5, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

int32 Room406::_state1;
int32 Room406::_state2;
int32 Room406::_state3;
int32 Room406::_state4;


Room406::Room406() : Section4Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
}

void Room406::init() {
	digi_preload("400_001");
	digi_play_loop("400_001", 3, 125);
	pal_cycle_init(119, 127, 10);
	_flag1 = false;

	if (_G(flags)[V172] != 10026) {
		loadSeries();

		if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
			switch (_G(flags)[V172]) {
			case 10023:
				_dogShould = 51;
				break;
			case 10024:
				_dogShould = 58;
				break;
			case 10025:
				_dogShould = 64;
				break;
			default:
				break;
			}

			if (_G(flags)[V172] != 10026)
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
		} else {
			_dogShould = imath_ranged_rand(1, 2) == 1 ? 49 : 50;

			if (_G(flags)[V172] == 10025) {
				_dogShould = 64;
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
		}
	}

	setHotspots1();
	_val2 = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		if (player_been_here(406) || _G(flags)[V176]) {
			if (_G(flags)[V172] != 10025 && _G(flags)[V172] != 10026) {
				_val2 = 23;
				kernel_trigger_dispatch_now(1);
			}
		} else {
			_val2 = 22;
			kernel_trigger_dispatch_now(1);
		}
	}

	_flag2 = !_G(flags)[V176];

	if (!_val2 && _flag2)
		kernel_timing_trigger(300, 9);

	if (_G(flags)[V171] == 4001)
		_coll.show("406coll", 0x4fd, 0, -1, -1, 2);
	
	setHotspots2();

	if (!_G(flags)[V175]) {
		_gateS1 = series_load("406gate");
		_gateS2 = series_load("406gateS");
		_gate.show("406gate", 0x4fd);
	}

	series_show("406tire", 0x702);
	_tire = 0;
	setupTt();
	setNoWalk();

	if (_G(flags)[V174] != 4003)
		_tts = series_show("406tts", 0x601);

	setHotspots4();
	setupFish();

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[V174] == 4003) {
			ws_hide_walker();
			_G(wilbur_should) = 7;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 402:
		ws_demand_location(44, 310);
		poofArrive();
		break;

	case 404:
		ws_demand_location(173, 302, 1);
		ws_walk(237, 308, nullptr, -1);
		break;

	case 405:
		ws_demand_location(460, 346);
		poofArrive();
		break;

	default:
		ws_demand_location(169, 336, 5);
		break;
	}
}

void Room406::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(false);

		switch (_val2) {
		case 22:
			switch (_dogShould) {
			case 49:
				_state2 = imath_ranged_rand(1, 7);
				_dogShould = 51;
				series_play_with_breaks(PLAY20, "406dg06", 0x701, 12, 3);
				break;

			case 50:
				_dogShould = 51;
				series_play_with_breaks(PLAY21, "406dg07", 0x701, 12, 3);
				break;

			default:
				break;
			}

			kernel_timing_trigger(15, 2);
			break;

		case 23:
			switch (_dogShould) {
			case 49:
				_state2 = imath_ranged_rand(1, 7);
				series_play_with_breaks(PLAY20, "406dg06", 0x701, 2, 3);
				break;

			case 50:
				series_play_with_breaks(PLAY21, "406dg07", 0x701, 2, 3);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 2:
		switch (_val2) {
		case 22:
			playRandom(3);
			break;

		case 23:
			_state2 = imath_ranged_rand(1, 6);
			series_play_with_breaks(PLAY22, "406dg16", 0x701, 3, 3);
			break;

		default:
			break;
		}
		break;

	case 3:
		switch (_val2) {
		case 22:
			kernel_timing_trigger(300, 9);
			_state4 = 22;
			ws_turn_to_face(calc_facing(162, 191), 8);
			break;

		case 23:
			_dogShould = 51;
			kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			kernel_timing_trigger(300, 9);
			_state4 = 23;
			ws_turn_to_face(calc_facing(415, 234), 8);
			break;

		default:
			break;
		}

		_val2 = 0;
		break;

	case 5:
		poof(6);
		break;

	case 6:
		disable_player_commands_and_fade_init(_val4);
		break;

	case 7:
		_G(walker).wilbur_poof();

		if (_val2)
			ws_unhide_walker();
		else
			enable_player();
		break;

	case 8:
		switch (_state4) {
		case 22:
			player_set_commands_allowed(true);
			wilbur_speech("406w001");
			break;

		case 23:
			player_set_commands_allowed(true);
			_G(walker).wilbur_speech_random("406w002a", "406w002b", "406w002c");
			break;

		case 24:
			wilbur_speech("400w001");
			break;

		case 25:
			wilbur_speech("406w013");
			break;

		case 26:
			wilbur_speech("406w014");
			break;

		case 27:
			wilbur_speech("406w015");
			break;

		case 28:
			term_message("The gate isn't open yet.  I can't get over there!");
			break;

		case 29:
			wilbur_speech("406w020");
			break;

		case 30:
			wilbur_speech("406w021");
			break;

		case 31:
			wilbur_speech("406w022");
			break;

		case 32:
			wilbur_speech("406w023");
			break;

		case 33:
			wilbur_speech("406w024");
			break;

		case 34:
			_G(wilbur_should) = 13;
			wilbur_speech("406w017", kCHANGE_WILBUR_ANIMATION);
			break;

		case 35:
			wilbur_speech("406w019");
			break;

		case 36:
			wilbur_speech("406w026");
			break;

		case 37:
			wilbur_speech("406w008");
			break;

		case 38:
			wilbur_speech("406w009");
			break;

		case 39:
			wilbur_speech("406w027");
			break;

		case 40:
			wilbur_speech("406w029");
			break;

		case 41:
			wilbur_speech("406w030");
			break;

		case 42:
			wilbur_speech("406w031");
			break;

		case 43:
			wilbur_speech("406w032");
			break;

		case 44:
			wilbur_speech("406w033");
			break;

		case 45:
			digi_unload("406_111");
			_G(wilbur_should) = 15;
			wilbur_speech("406w018", kCHANGE_WILBUR_ANIMATION);
			break;

		case 46:
			_mayorShould = 68;
			wilbur_speech("406w901", 11);
			break;

		case 47:
			_mayorShould = 71;
			_state1 = 80;
			wilbur_speech("406w902", 11);
			break;

		case 48:
			wilbur_speech("406w038");
			break;

		default:
			break;
		}
		break;

	case 9:
		if (_flag2) {
			_flag3 = false;
			playRandom(-1);
			_ticks = imath_ranged_rand(300, 600);
			kernel_timing_trigger(_ticks - 120, 10);
			kernel_timing_trigger(_ticks, 9);
		}
		break;

	case 10:
		_flag3 = true;
		break;

	case kCHANGE_MAYOR_ANIMATION:
		switch (_mayorShould) {
		case 68:
			_mayorShould = 69;
			digi_play("406g002", 1, 255, kCHANGE_MAYOR_ANIMATION);
			break;

		case 69:
			_mayorShould = 70;
			series_play_with_breaks(PLAY11, "406mg01", 0x5ff, kCHANGE_MAYOR_ANIMATION, 3);
			break;

		case 70:
			_state1 = 79;
			_mayorShould = 71;
			series_play_with_breaks(PLAY12, "406mg03", 0x5ff, kCHANGE_MAYOR_ANIMATION, 3);
			break;

		case 71:
			_mg03.terminate();

			switch (_state1) {
			case 79:
				_mayorShould = 72;
				break;
			case 80:
				_mayorShould = 73;
				break;
			case 81:
				_mayorShould = 74;
				break;
			case 82:
				_mayorShould = 72;
				break;
			default:
				break;
			}

			series_play_with_breaks(PLAY13, "406mg04", 0x5ff, kCHANGE_MAYOR_ANIMATION, 3);
			break;

		case 72:
			_mg03.show("406mg03", 0x5ff);
			_state4 = 47;
			kernel_trigger_dispatch_now(8);
			break;

		case 73:
			_mayorShould = 71;
			_state1 = 81;
			series_play_with_breaks(PLAY14, "406mg05", 0x5ff, kCHANGE_MAYOR_ANIMATION, 3);
			break;

		case 74:
			_dogShould = 67;
			_mg03.show("406mg03", 0x5ff, 0, -1, -1, 14);
			break;

		case 75:
			_mg03.terminate();
			_mayorShould = 76;
			series_play_with_breaks(PLAY15,
				_G(flags)[V171] == 4000 ? "406mg06" : "406mg07",
				0x5ff, kCHANGE_MAYOR_ANIMATION, 3);
			break;

		case 76:
			_val6 = 85;
			_mayorShould = 77;

			if (_G(flags)[V171] == 4000)
				series_play_with_breaks(PLAY16, "406mg06", 0x6fe, kCHANGE_MAYOR_ANIMATION, 3);
			else
				series_play_with_breaks(PLAY17, "406mg07", 0x6fe, kCHANGE_MAYOR_ANIMATION, 3);
			break;

		case 77:
			_G(flags)[V176] = 1;
			_G(flags)[V172] = 10026;

			if (_G(flags)[V171] == 4000)
				_G(flags)[V171] = 4001;

			setHotspots1();
			_mayorShould = 78;
			series_play_with_breaks(PLAY18, "406mgpof", 0x6fe, kCHANGE_MAYOR_ANIMATION, 2);
			_val6 = 77;
			kernel_trigger_dispatch_now(15);
			break;

		case 78:
			hotspot_set_active("HOLE", true);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_DOG_ANIMATION:
		switch (_dogShould) {
		case 51:
			_G(flags)[V172] = 10023;
			setHotspots1();

			if (_G(flags)[V173]) {
				_dogShould = 62;
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			} else {
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					_dogShould = 51;
					break;
				case 2:
					_dogShould = 55;
					break;
				case 3:
					_dogShould = 53;
					break;
				case 4:
					_dogShould = 54;
					break;
				default:
					break;
				}

				Series::series_show("406dg15", 0x701, 0, kCHANGE_DOG_ANIMATION, 120, 0);
			}
			break;

		case 53:
			_state2 = imath_ranged_rand(1, 7);
			_dogShould = 51;

			if (!digi_play_state(1) && !digi_play_state(2)) {
				series_play_with_breaks(PLAY23, "406dg08", 0x701, kCHANGE_DOG_ANIMATION, 3);
			} else {
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
			break;

		case 54:
			if (!digi_play_state(1) && !digi_play_state(2)) {
				_G(flags)[V172] = 10024;
				_state2 = imath_ranged_rand(1, 3);
				_dogShould = 58;
				series_play_with_breaks(PLAY24, "406dg10", 0x701, kCHANGE_DOG_ANIMATION, 3);
			} else {
				_dogShould = 51;
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
			break;

		case 55:
			_state2 = imath_ranged_rand(1, 6);
			_dogShould = 51;

			if (!digi_play_state(1) && !digi_play_state(2)) {
				series_play_with_breaks(PLAY22, "406dg16", 0x701, kCHANGE_DOG_ANIMATION, 3);
			} else {
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
			break;

		case 56:
			_state2 = imath_ranged_rand(1, 6);
			_dogShould = 58;

			if (!digi_play_state(1) && !digi_play_state(2)) {
				series_play_with_breaks(PLAY22, "406dg12", 0x701, kCHANGE_DOG_ANIMATION, 3);
			} else {
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
			break;

		case 57:
			if ((!digi_play_state(1) && !digi_play_state(2)) || _G(flags)[V173]) {
				_state2 = imath_ranged_rand(1, 3);
				_dogShould = 51;
				_G(flags)[V172] = 10023;
				series_play_with_breaks(PLAY25, "406dg14", 0x701, kCHANGE_DOG_ANIMATION, 3);
			} else {
				_dogShould = 58;
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
			break;

		case 58:
			_G(flags)[V172] = 10024;
			setHotspots1();

			if (_G(flags)[V173]) {
				_dogShould = 57;
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			} else {
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					_dogShould = 58;
					break;
				case 2:
					_dogShould = 57;
					break;
				case 3:
					_dogShould = 56;
					break;
				default:
					break;
				}

				Series::series_show("406dg11", 0x701, 0, kCHANGE_DOG_ANIMATION, 120);
			}
			break;

		case 59:
			_state2 = imath_ranged_rand(1, 6);
			_dogShould = 54;
			series_play_with_breaks(PLAY26, "406dg09", 0x701, kCHANGE_DOG_ANIMATION, 3);
			break;

		case 60:
			_state2 = imath_ranged_rand(1, 6);
			_dogShould = 56;
			series_play_with_breaks(PLAY27, "406dg13", 0x701, kCHANGE_DOG_ANIMATION, 3);
			break;

		case 61:
			_state2 = imath_ranged_rand(1, 6);
			_dogShould = 64;
			series_play_with_breaks(PLAY28,
				_G(flags)[V171] == 4000 ? "406dg04" : "406dg05", 0x6fe, kCHANGE_DOG_ANIMATION, 3);
			break;

		case 62:
			_G(flags)[V172] = 10025;
			_G(flags)[V173] = 0;
			setHotspots1();
			terminateMachineAndNull(_fish);
			series_unload(_fishS);
			_state2 = imath_ranged_rand(1, 4);
			_state3 = imath_ranged_rand(1, 4);
			_dogShould = 63;
			series_play_with_breaks(PLAY29, "406dg01", 0x6fe, kCHANGE_DOG_ANIMATION, 3);
			break;

		case 63:
			setupFish();
			setHotspots1();
			_dogShould = 64;
			kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			hotspot_set_active("HOLE", false);
			break;

		case 64:
			_dogShould = imath_ranged_rand(1, 3) == 1 ? 66 : 64;
			_state2 = imath_ranged_rand(1, 6);

			if (digi_play_state(2) || _flag3)
				_state2 = 0;

			_seriesName = _G(flags)[V171] == 4000 ? "406dg02" : "406dg03";
			series_play_with_breaks(PLAY30, _seriesName, 0x6fe, kCHANGE_DOG_ANIMATION, 3, 10);
			break;

		case 65:
			_G(wilbur_should) = 21;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			_dogShould = 64;
			kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			break;

		case 66:
			_dogShould = 64;
			_state2 = imath_ranged_rand(1, 5);

			if (!digi_play_state(1) && !digi_play_state(2)) {
				const seriesPlayBreak *PLAY[6] = {
					PLAY31, PLAY32, PLAY33, PLAY34, PLAY35, PLAY36
				};

				_seriesName = _G(flags)[V171] == 4000 ? "406dg02" : "406dg03";
				series_play_with_breaks(PLAY[imath_ranged_rand(0, 5)],
					_seriesName, 0x6fe, kCHANGE_DOG_ANIMATION, 3);
			} else {
				kernel_trigger_dispatch_now(kCHANGE_DOG_ANIMATION);
			}
			break;

		case 67:
			_mayorShould = 75;
			kernel_trigger_dispatch_now(11);
			break;

		default:
			break;
		}
		break;

	case 13:
		digi_play("406_113", 2);
		break;

	case 14:
		switch (_val7) {
		case 82:
			terminateMachineAndNull(_tt);
			_G(flags)[V174] = 4001;
			_val7 = 83;
			series_play_with_breaks(PLAY10, "406tt06", 0x600, 14, 2);
			break;

		case 83:
			player_set_commands_allowed(true);
			setupTt();
			break;

		default:
			break;
		}
		break;

	case 15:
		switch (_val6) {
		case 77:
			terminateMachineAndNull(_fish);
			series_play_with_breaks(PLAY19, "406fishp", 0x4fd, -1, 2);
			break;

		case 84:
			_G(flags)[V173] = 1;
			setupFish();
			inv_move_object("FISH", NOWHERE);

			if (_G(flags)[V172] == 10024)
				_dogShould = 57;
			if (_G(flags)[V172] == 10023)
				_dogShould = 62;
			break;

		case 85:
			_fish = series_play("406fish2", 0x4fd, 16);
			break;

		default:
			break;
		}
		break;

	case 16:
		_state4 = 45;
		digi_play("406_111", 1, 255, 8);
		break;

	case 17:
		_coll.play("406coll", 0x4fd, 16);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			ws_unhide_walker();
			break;

		case 2:
			disable_player();
			_val6 = 84;
			_fishS = series_load("406fish");
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY2, "406wi05", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			disable_player();
			_val7 = 82;
			kernel_trigger_dispatch_now(14);
			_G(wilbur_should) = 1;
			series_play_with_breaks(PLAY4, "406wi03", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			terminateMachineAndNull(_tt);
			disable_player();
			_G(wilbur_should) = 5;
			series_play_with_breaks(PLAY5, "406tt6a", 0x4fd, kCHANGE_WILBUR_ANIMATION, 2);
			_wi03 = series_play("406wi03s", 0x4fe, 18);
			break;

		case 5:
			terminateMachineAndNull(_wi03);
			enable_player();
			_G(flags)[V174] = 4000;
			setupTt();
			break;

		case 6:
			terminateMachineAndNull(_tt);
			disable_player();
			_G(wilbur_should) = 7;
			series_play_with_breaks(PLAY6, "406wi04", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 7:
			player_set_commands_allowed(true);
			_general.show("406wi04", 0x4fd);
			inv_give_to_player("HOOK");
			_G(flags)[V174] = 4003;
			setHotspots3();
			break;

		case 8:
			_general.terminate();
			disable_player();
			_G(wilbur_should) = 9;
			series_play_with_breaks(PLAY7, "406wi04", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 9:
			inv_move_object("HOOK", NOWHERE);
			_G(flags)[V174] = 4001;
			setupTt();
			enable_player();

			if (!player_said("HOOK") || !player_said_any("YARD", "YARD "))
				triggerPreparser();
			break;

		case 11:
			_general.terminate();
			_G(flags)[V174] = 4002;
			ws_demand_location(183, 281, 10);
			disable_player();
			_G(wilbur_should) = 12;
			series_play_with_breaks(PLAY8, "406wi04", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 12:
			inv_move_object("HOOK", NOWHERE);
			setupTt();
			enable_player();
			setNoWalk();
			break;

		case 13:
			terminateMachineAndNull(_tt);
			_G(flags)[V174] = 4004;
			_flag2 = false;
			digi_preload("406_111");
			disable_player();

			_G(wilbur_should) = 14;
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "406tt04", 6, 0x600, kCHANGE_WILBUR_ANIMATION);

			_wi03 = series_play("406wi03s", 0x601, 18);
			break;

		case 14:
			digi_unload_stream_breaks(SERIES1);
			setupTt();
			setHotspots4();
			intr_remove_no_walk_rect(_walk1);
			intr_remove_no_walk_rect(_walk2);
			setNoWalk();

			_flag2 = _flag3 = false;
			terminateMachineAndNull(_wi03);
			ws_unhide_walker();
			break;

		case 15:
			_state4 = 46;
			ws_walk(292, 333, nullptr, 8, 10);
			break;

		case 16:
			if (player_commands_allowed()) {
				player_set_commands_allowed(false);
				_flag2 = false;
				_flag3 = true;
			}

			if (digi_play_state(2)) {
				kernel_timing_trigger(15, kCHANGE_WILBUR_ANIMATION);
			} else {
				_gate.terminate();
				series_unload(_gateS1);
				series_unload(_gateS2);
				ws_hide_walker();

				_G(flags)[V175] = 1;
				_G(flags)[V177] = 1;
				setHotspots2();

				_G(wilbur_should) = 17;
				series_play_with_breaks(PLAY1, "406wi01", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 17:
			enable_player();
			_flag2 = true;
			_flag3 = false;
			kernel_trigger_dispatch_now(9);
			break;

		case 18:
			disable_player();
			terminateMachineAndNull(_coll406);
			_G(wilbur_should) = 19;
			series_play_with_breaks(PLAY3, "406wi02", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 19:
			enable_player();
			inv_give_to_player("DOG COLLAR");
			_G(flags)[V171] = 4002;
			setHotspots1();
			break;

		case 20:
			player_set_commands_allowed(false);
			_G(flags)[V171] = 4002;
			_dogShould = 65;
			break;

		case 21:
			ws_hide_walker();
			_G(wilbur_should) = 19;
			series_play_with_breaks(PLAY9, "406wi06", 0x4fd, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case 10008:
		switch (_G(flags)[V172]) {
		case 10023:
			_dogShould = 59;
			break;
		case 10024:
			_dogShould = 60;
			break;
		case 10025:
			_dogShould = 61;
			break;
		default:
			break;
		}
		break;

	case kSET_COMMANDS_ALLOWED:
		if (!_val2)
			player_set_commands_allowed(true);
		break;

	case kCALLED_EACH_LOOP:
		if (_flag1)
			parseJail();
		else
			_G(kernel).call_daemon_every_loop = false;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room406::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (inv_player_has("HOOK")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = false;

		if (player_said("HOOK", "BARRED WINDOW ")) {
			_G(wilbur_should) = 11;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else if (!player_said("HOOK") || player_said_any("YARD", "YARD ")) {
			_G(wilbur_should) = 8;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			g_engine->parse_player_command_now();
		}

		_G(player).command_ready = false;

	} else if (player_said("GATE") && _G(flags)[V172] == 10025) {
		player_hotspot_walk_override(280, 296, 2);
	} else if (player_said("GEAR") && player_said_any("WINCH", "LEVER", "LEVER ") && _G(flags)[V175]) {
		player_hotspot_walk_override(546, 284, 9);
	} else if (player_said("TAKE", "DOG COLLAR   ") || player_said("FISH", "TIRE")) {
		player_hotspot_walk_override(248, 298, 1);
	} else if (player_said_any("CHAIN    ", "CHAIN   ")) {
		player_walk_to(340, 300);
	} else if (player_said("GEAR", "DISC")) {
		_val4 = 4007;
		player_hotspot_walk_override(44, 310, -1, 5);
		_G(player).command_ready = false;
	} else if (player_said("GEAR", "DISC ")) {
		_val4 = 4007;
		player_hotspot_walk_override(460, 346, -1, 5);
		_G(player).command_ready = false;
	} else if (player_said("LOOK AT") && player_said_any("JAIL CELL", "JAIL CELL ") && _G(flags)[V174] == 4004) {
		if (!_flag1) {
			parseJail();
			_G(kernel).call_daemon_every_loop = true;

			if (player_said("LOOK AT", "JAIL CELL"))
				player_hotspot_walk_override(173, 302, 10);
		}

		_G(player).command_ready = false;
	} else {
		if (_G(flags)[V174] == 4002) {
			if (_G(player).walk_x >= 180 && _G(player).walk_x <= 309 &&
				_G(player).walk_y >= 271 && _G(player).walk_y <= 280)
				player_walk_to(_G(player).walk_x, 281);

			if (_G(player).walk_x >= 350 && _G(player).walk_x <= 409 &&
				_G(player).walk_y >= 263 && _G(player).walk_y <= 283)
				player_walk_to(_G(player).walk_x, 284);
		}

		if (_G(flags)[V174] == 4004) {
			if (_G(player).walk_x >= 336 && _G(player).walk_x <= 388 &&
				_G(player).walk_y >= 272 && _G(player).walk_y <= 291)
				player_walk_to(_G(player).walk_x, 292);

			if (_G(player).walk_x >= 378 && _G(player).walk_x <= 409 &&
				_G(player).walk_y >= 259 && _G(player).walk_y <= 291)
				player_walk_to(_G(player).walk_x, 292);
		}

		if (player_said("FORCE FIELD")) {
			player_set_facing_hotspot();
		} else if (!_G(flags)[V175]) {
			_hotspot = hotspot_which(_G(player).click_x, _G(player).click_y);
			assert(_hotspot);

			if (_hotspot->feet_y == 0x7fff) {
				term_message("click_y: %d     taboo_area_y (click_x)",
					_G(player).click_x, tabooAreaY(_G(player).click_x));
			} else {
				term_message("feet_y: %d     taboo_area_y (feet_x): %d",
					_hotspot->feet_y, tabooAreaY(_hotspot->feet_x));
			}

			if (_hotspot->feet_x > _hotspot->feet_y ||
					(_hotspot->feet_y == 0x7fff && tabooAreaY(_G(player).click_x) > _G(player).click_y))
				player_walk_to(_hotspot->feet_x, tabooAreaY(_hotspot->feet_x) + 1);
		}
	}
}

void Room406::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID) || _G(walker).wilbur_match(MATCH))
		_G(player).command_ready = false;
}

void Room406::loadSeries() {
	if (_G(flags)[V172] == 10025) {
		series_load("406dg02");
		series_load("406dg02s");
	}

	if (_G(flags)[V172] == 10023 || _G(flags)[V172] == 10024) {
		if (_dogShould == 49) {
			series_load("406dg06");
			series_load("406dg06s");
		}

		if (_dogShould == 50) {
			series_load("406dg07");
			series_load("406dg07s");
		}

		series_load("406dg11");
		series_load("406dg11s");
		series_load("406dg15");
		series_load("406dg15s");
	}			
}

void Room406::setHotspots1() {
	hotspot_set_active("PEGLEG", false);
	hotspot_set_active("PEGLEG ", false);
	hotspot_set_active("PEGLEG  ", false);
	hotspot_set_active("DOG COLLAR ", false);
	hotspot_set_active("DOG COLLAR  ", false);
	hotspot_set_active("DOG COLLAR   ", false);
	hotspot_set_active("DOG COLLAR    ", false);

	switch (_G(flags)[V172]) {
	case 10023:
		hotspot_set_active("PEGLEG", true);
		hotspot_set_active("DOG COLLAR ", true);
		break;

	case 10024:
		hotspot_set_active("PEGLEG ", true);
		hotspot_set_active("DOG COLLAR  ", true);
		break;

	case 10025:
		if (!_G(flags)[V173]) {
			hotspot_set_active("PEGLEG  ", true);
			if (_G(flags)[V171] == 4000)
				hotspot_set_active("DOG COLLAR   ", true);
		}

		hotspot_set_active("HOLE", false);
		break;

	case 10026:
		if (_G(flags)[V171] == 4001)
			hotspot_set_active("DOG COLLAR    ", true);

	default:
		break;
	}
}

void Room406::setHotspots2() {
	if (_G(flags)[V175]) {
		hotspot_set_active("GATE", false);
		hotspot_set_active("LOCK", false);
		hotspot_set_active("CHAIN", false);
		hotspot_set_active("RAZOR WIRE ", false);
		hotspot_set_active("YARD ", true);
	} else {
		hotspot_set_active("GATE", true);
		hotspot_set_active("LOCK", true);
		hotspot_set_active("CHAIN", true);
		hotspot_set_active("RAZOR WIRE ", true);
		hotspot_set_active("YARD ", false);
	}
}

void Room406::setHotspots3() {
	hotspot_set_active("HOOK ", false);
	hotspot_set_active("HOOK  ", false);
	hotspot_set_active("HOOK   ", false);
	hotspot_set_active("HOOK    ", false);
	hotspot_set_active("CHAIN ", false);
	hotspot_set_active("CHAIN  ", false);
	hotspot_set_active("CHAIN   ", false);
	hotspot_set_active("CHAIN    ", false);
	hotspot_set_active("LEVER", false);
	hotspot_set_active("LEVER ", false);

	switch (_G(flags)[V174]) {
	case 4000:
		hotspot_set_active("HOOK ", true);
		hotspot_set_active("CHAIN ", true);
		hotspot_set_active("LEVER", true);
		break;

	case 4001:
		hotspot_set_active("HOOK  ", true);
		hotspot_set_active("CHAIN  ", true);
		hotspot_set_active("LEVER ", true);
		break;

	case 4002:
		hotspot_set_active("HOOK   ", true);
		hotspot_set_active("CHAIN   ", true);
		hotspot_set_active("LEVER ", true);
		break;

	case 4003:
		hotspot_set_active("CHAIN ", true);
		hotspot_set_active("LEVER ", true);
		break;

	case 4004:
		hotspot_set_active("HOOK    ", true);
		hotspot_set_active("CHAIN    ", true);
		hotspot_set_active("LEVER", true);
		break;

	default:
		break;
	}
}

void Room406::setHotspots4() {
	hotspot_set_active("JAIL CELL ", false);
	hotspot_set_active("RUBBLE", false);
	hotspot_set_active("BARRED WINDOW  ", false);
	hotspot_set_active("HOOK    ", false);

	if (_G(flags)[V174] == 4004) {
		hotspot_set_active("JAIL CELL ", true);
		hotspot_set_active("RUBBLE", true);
		hotspot_set_active("BARRED WINDOW ", false);
		hotspot_set_active("BARRED WINDOW  ", true);
		hotspot_set_active("HOOK    ", true);
	}
}

void Room406::setupFish() {
	hotspot_set_active("FISH ", false);
	hotspot_set_active("FISH  ", false);

	if (_G(flags)[V173]) {
		_fish = series_show("406fish", 0x4fd);
		hotspot_set_active("FISH ", true);
	}

	if (_G(flags)[V172] == 10025)
		hotspot_set_active("FISH  ", true);
}

void Room406::setupTt() {
	if (_tt)
		terminateMachineAndNull(_tt);
	if (_tire)
		series_unload(_tire);

	switch (_G(flags)[V174]) {
	case 4000:
		_tire = series_load("406tt");
		_tt = series_show("406tt", 0x600);
		break;

	case 4001:
		_tire = series_load("406tt02");
		_tt = series_show("406tt02", 0x600);
		break;

	case 4002:
		_tire = series_load("406tt03");
		_tt = series_show("406tt03", 0x600);
		break;

	case 4004:
		_tire = series_load("406tt05");
		_tt = series_show("406tt05", 0x600);
		break;

	default:
		break;
	}

	setHotspots3();
}

void Room406::setNoWalk() {
	if (_G(flags)[V174] == 4002) {
		_walk1 = intr_add_no_walk_rect(180, 271, 309, 280, 179, 281);
		_walk2 = intr_add_no_walk_rect(350, 263, 409, 283, 349, 284);
	}

	if (_G(flags)[V174] == 4004) {
		_walk3 = intr_add_no_walk_rect(336, 272, 388, 291, 335, 292);
		_walk4 = intr_add_no_walk_rect(378, 259, 409, 291, 377, 292);
	}
}

void Room406::parseJail() {
	if (player_said("LOOK AT") && player_said_any("JAIL CELL", "JAIL CELL "))
		_flag1 = true;

	if (_flag1) {
		player_update_info();

		if (_G(player_info).x < 183 && _G(player_info).y < 312 && player_commands_allowed())
			disable_player_commands_and_fade_init(4003);
	}
}

int Room406::tabooAreaY(int x) const {
	return ((double)x * -0.1050583 + -240.21069) * -1.0;
}

void Room406::playRandom(int trigger) {
	digi_unload(_randomDigi);

	if (!digi_play_state(2)) {
		_randomDigi = Common::String::format("406g001%c", 'a' + imath_ranged_rand(0, 18));
		digi_preload(_randomDigi);
		digi_play(_randomDigi.c_str(), 2, 125, trigger);
	}
}

void Room406::triggerPreparser() {
	_G(player).waiting_for_walk = true;
	_G(player).ready_to_walk = true;
	_G(player).need_to_walk = true;
	_G(player).command_ready = true;
	_G(kernel).trigger = -1;
	_G(kernel).trigger_mode = KT_PREPARSE;
	_G(player).walker_trigger = -1;

	pre_parser();
	if (_G(player).command_ready) {
		g_engine->_activeSection->pre_parser();

		if (_G(player).command_ready)
			g_engine->global_pre_parser();
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
