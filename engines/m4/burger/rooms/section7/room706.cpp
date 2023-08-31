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

#include "m4/burger/rooms/section7/room706.h"
#include "m4/burger/rooms/section7/section7.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak SERIES1[] = {
	{   0, "706F001",  1, 255, -1, 0, nullptr, 0 },
	{  31, "706W001",  1, 255, -1, 0, nullptr, 0 }, 
	{  52, "706F002",  1, 255, -1, 0, nullptr, 0 }, 
	{  83, "706A_001", 2, 150, -1, 0, nullptr, 0 }, 
	{ 105, "706A_002", 2, 125, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{   0, "706w002",  1, 255, -1, 0, nullptr, 0 },
	{  50, "706F003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 103, "706W003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 140, "706F004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 145, "706A_003", 2, 200, -1, 0, nullptr, 0 }, 
	{ 170, "706W004",  1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{   3, "707A_002", 1, 125, -1, 0, nullptr, 0 },
	{  11, "707A_001", 2, 175, -1, 0, nullptr, 0 }, 
	{  32, "707W001",  1, 255, -1, 0, nullptr, 0 }, 
	{  56, "707F001",  1, 255, -1, 0, nullptr, 0 }, 
	{ 100, "707W002",  1, 255, -1, 0, nullptr, 0 }, 
	{ 104, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 115, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 120, "707F002",  1, 255, -1, 0, nullptr, 0 }, 
	{ 134, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 147, "707W003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 163, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 180, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 190, "707F003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 192, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 201, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 214, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 222, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 233, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 237, "707W004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 239, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 253, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 259, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 267, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 268, "707F004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 271, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 277, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 281, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 295, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 303, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 317, "707F005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 323, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 345, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 347, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 365, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 379, "707Z001",  1, 255, -1, 0, nullptr, 0 }, 
	{ 389, "707Z002",  1, 255, -1, 0, nullptr, 0 }, 
	{ 450, "707Z003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 473, "707F006",  1, 255, -1, 0, nullptr, 0 }, 
	{ 478, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 491, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 509, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 523, "707A_003", 2,  50, -1, 0, nullptr, 0 }, 
	{ 529, "707A_007", 2, 255, -1, 0, nullptr, 0 }, 
	{ 535, "707Z005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 536, "707A_007", 2, 255, -1, 0, nullptr, 0 }, 
	{ 543, "707Z004a", 1, 255, -1, 0, nullptr, 0 }, 
	{ 544, "707A_007", 2, 255, -1, 0, nullptr, 0 }, 
	{ 550, "707Z005a", 1, 255, -1, 0, nullptr, 0 }, 
	{ 556, "707Z007",  1, 255, -1, 0, nullptr, 0 }, 
	{ 589, "707F007",  1, 255, -1, 0, nullptr, 0 }, 
	{ 618, "707Z008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 645, "707F008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 686, "707F009",  1, 255, -1, 0, nullptr, 0 }, 
	{ 710, "707W005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 711, "707A_006", 2, 100, -1, 0, nullptr, 0 }, 
	{ 718, "707A_006", 2, 100, -1, 0, nullptr, 0 }, 
	{ 725, "707A_006", 2, 100, -1, 0, nullptr, 0 }, 
	{ 729, "707Z009",  1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES4[] = {
	{   0, "708Z001", 1, 255, -1, 0, nullptr, 0 },
	{  48, "708W001", 1, 255, -1, 0, nullptr, 0 },  
	{ 107, "708Z002", 1, 255, -1, 0, nullptr, 0 }, 
	{ 187, "708W002", 1, 255, -1, 0, nullptr, 0 }, 
	{ 206, "708Z003", 1, 255, -1, 0, nullptr, 0 }, 
	{ 296, "708A001", 1, 200, -1, 0, nullptr, 0 }, 
	{ 312, "708Z004", 1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES5[] = {
	{ 0, "707A001", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES6[] = {
	{ 0, "708Z005", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES7[] = {
	{   0, "707C_001", 3, 100, -1, (uint)-1, nullptr, 0 },
	{   1, nullptr,    0,   0, 37,  0, nullptr, 0 },
	{   2, "707A002",  1, 255, -1,  0, nullptr, 0 }, 
	{  33, "707Z010",  1, 255, -1,  0, nullptr, 0 },  
	{  95, "707C_002", 2, 200, -1,  0, nullptr, 0 },  
	{ 100, "707Z011",  1, 255, -1,  0, nullptr, 0 }, 
	{ 124, "707F010",  1, 255, -1,  0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES8[] = {
	{  0, "709A_004", 3,  75, -1, (uint)-1, nullptr, 0 },
	{  0, "709W001",  1, 255, -1,  0, nullptr, 0 }, 
	{  5, "709A_002", 2, 110, -1,  0, nullptr, 0 }, 
	{ 16, "709A001",  1, 255, -1,  0, nullptr, 0 },
	{ 41, "709A_003", 2,  75, -1,  0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES9[] = {
	{   0, "707D_005", 3, 75, -1, (uint)-1, nullptr, 0 },
	{   0, "707W006",  1, 255, -1, 0, nullptr, 0 },
	{  16, "707W007",  1, 255, -1, 0, nullptr, 0 },
	{  28, nullptr,    0,   0, 35, 0, nullptr, 0 },
	{  29, "707D_001", 2, 150, -1, 0, nullptr, 0 },
	{  31, "707A003",  1, 255, -1, 0, nullptr, 0 },
	{  41, "707A004",  1, 255, -1, 0, nullptr, 0 }, 
	{  84, "707W008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 126, "707W009",  1, 255, -1, 0, nullptr, 0 }, 
	{ 132, nullptr,    0,   0, 36, 0, nullptr, 0 },
	{ 134, "707D_006", 3, 100, -1, 0, nullptr, 0 },
	{ 138, "707A_003", 2,  50, -1, 0, nullptr, 0 },  
	{ 141, "707A_003", 2,  50, -1, 0, nullptr, 0 },  
	{ 143, "707A_002", 2, 125, -1, 0, nullptr, 0 }, 
	{ 158, "707A005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 175, "707W010",  1, 255, -1, 0, nullptr, 0 }, 
	{ 223, "707W010z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 320, "707A006",  1, 255, -1, 0, nullptr, 0 }, 
	{ 353, "707W011",  1, 125, -1, 0, nullptr, 0 }, 
	{ 364, "707W012",  1, 255, -1, 0, nullptr, 0 }, 
	{ 453, "707W013",  1, 125, -1, 0, nullptr, 0 }, 
	{ 475, "707A007",  1, 255, -1, 0, nullptr, 0 }, 
	{ 600, "707W014",  1, 255, -1, 0, nullptr, 0 }, 
	{ 601, nullptr,    0,   0, 38, 0, nullptr, 0 },
	{ 650, "707A008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 676, "707W015",  1, 255, -1, 0, nullptr, 0 }, 
	{ 696, "707W016",  1, 255, -1, 0, nullptr, 0 }, 
	{ 698, "707D_008", 2, 200, -1, 0, nullptr, 0 }, 
	{ 715, "707D_004", 2, 150, -1, 0, nullptr, 0 }, 
	{ 729, "707A009",  1, 255, -1, 0, nullptr, 0 }, 
	{ 730, "707D_008", 2, 200, -1, 0, nullptr, 0 }, 
	{ 753, "707W017",  1, 255, -1, 0, nullptr, 0 }, 
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES10[] = {
	{  0, "709A002",   1, 255, -1,    0, nullptr, 0 },
	{  45, "709B_001", 2, 100, -1,    0, nullptr, 0 }, 
	{  56, "709A003",  1, 255, -1,    0, nullptr, 0 }, 
	{  83, "709W002",  2, 255, -1,    0, nullptr, 0 }, 
	{  92, "709A004",  1, 255, -1,    0, nullptr, 0 }, 
	{  95, "709B_002", 2, 255, -1,    0, nullptr, 0 }, 
	{ 103, "709W003",  1, 255, -1,    0, nullptr, 0 }, 
	{ 110, nullptr,    0,   0, 39,    0, nullptr, 0 },
	{ 115, nullptr,    0,   0, 40,    0, nullptr, 0 },
	{ 120, "709B_006", 2, 255, -1,    0, nullptr, 0 }, 
	{ 124, "709B_004", 3,  50, -1, 1024, nullptr, 0 }, 
	{ 264, nullptr,    0,   0, 41,    0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room706::init() {
}

void Room706::daemon() {
}

void Room706::pre_parser() {
}

void Room706::parser() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
