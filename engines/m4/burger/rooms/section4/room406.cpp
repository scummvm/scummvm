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
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

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
};

long Room406::_state1;
long Room406::_state2;
long Room406::_state3;
long Room406::_state4;


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

		if (_G(game).previous_room == RESTORING_GAME) {
			switch (_G(flags)[V172]) {
			case 10023:
				_val1 = 51;
				break;
			case 10024:
				_val1 = 58;
				break;
			case 10025:
				_val1 = 64;
				break;
			default:
				break;
			}

			if (_G(flags)[V172] == 10026)
				kernel_trigger_dispatch_now(12);
		} else {
			_val1 = imath_ranged_rand(1, 2) == 1 ? 49 : 50;

			if (_G(flags)[V172] == 10025) {
				_val1 = 64;
				kernel_trigger_dispatch_now(12);
			}
		}
	}

	setHotspots1();
	_val2 = 0;

	if (_G(game).previous_room != RESTORING_GAME) {
		if (player_been_here(406) || _G(flags)[V176]) {
			if (_G(flags)[V172] != 10025 && _G(flags)[V172] != 10026) {
				_val2 = 23;
				kernel_trigger_dispatch_now(1);
			}
		} else {
			_val2 = 22;
			kernel_trigger_dispatch_now(22);
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
	case RESTORING_GAME:
		if (_G(flags)[V174] == 4003) {
			ws_hide_walker();
			_G(wilbur_should) = 7;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 402:
		ws_demand_location(44, 310);
		setDest(120);
		break;

	case 404:
		ws_demand_location(173, 302, 1);
		ws_walk(237, 308, nullptr, -1);
		break;

	case 405:
		ws_demand_location(460, 346);
		setDest(120);
		break;

	default:
		ws_demand_location(169, 336, 5);
		break;
	}
}

void Room406::daemon() {
}

void Room406::loadSeries() {
	if (_G(flags)[V172] == 10025) {
		series_load("406dg02");
		series_load("406dg02s");
	}

	if (_G(flags)[V172] == 10023 || _G(flags)[V172] == 10024) {
		if (_val1 == 49) {
			series_load("406dg06");
			series_load("406dg06s");
		}

		if (_val1 == 50) {
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
		hotspot_set_active("YARD", true);
	} else {
		hotspot_set_active("GATE", true);
		hotspot_set_active("LOCK", true);
		hotspot_set_active("CHAIN", true);
		hotspot_set_active("RAZOR WIRE ", true);
		hotspot_set_active("YARD", false);
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
		hotspot_set_active("HOOK ", true);
		hotspot_set_active("CHAIN ", true);
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

} // namespace Rooms
} // namespace Burger
} // namespace M4
