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

#include "m4/burger/rooms/section3/room301.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room301::SERIES1[] = {
	{   0, nullptr,    0,   0, 16,  0, nullptr, 0 },
	{   1, "301_010",  3,  60, -1, (uint)-1, nullptr, 0 },
	{   9, "301W001",  1, 255, -1,  0, nullptr, 0 }, 
	{  39, "301F001",  1, 255, -1,  0, nullptr, 0 },
	{  42, "301_007",  2, 255, -1,  0, nullptr, 0 }, 
	{  63, "301_003",  1, 255, -1,  0, nullptr, 0 }, 
	{  65, "301_019",  2, 275, -1,  0, nullptr, 0 }, 
	{  75, nullptr,    1,   0, -1,  0, nullptr, 0 },
	{  77, "301F001A", 1, 255, -1,  0, nullptr, 0 },
	{ 110, "301Z001",  1, 255, -1,  0, nullptr, 0 },
	{ 129, nullptr,    0,   0, 15,  0, nullptr, 0 },
	{ 132, nullptr,    0,   0, 20,  0, nullptr, 0 },
	{ 133, nullptr,    3,   0, -1,  0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room301::SERIES2[] = {
	{   0, nullptr,    0,   0, 17, 0, nullptr, 0 },
	{   1, "301_010",  3,  60, -1, (uint)-1, nullptr, 0 },
	{  12, "301Z002x", 1, 255, -1, 0, nullptr, 0 }, 
	{  17, "301_002",  2, 125, -1, 0, nullptr, 0 }, 
	{  31, "301Z002y", 1, 255, -1, 0, nullptr, 0 }, 
	{ 105, "301Z002z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 136, "301W003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 161, "301Z003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 301, "301F003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 335, "301Z005x", 1, 255, -1, 0, nullptr, 0 }, 
	{ 372, "301Z005y", 1, 255, -1, 0, nullptr, 0 }, 
	{ 385, "301Z005z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 434, "301_017",  2, 150, -1, 0, nullptr, 0 }, 
	{ 438, "301Z006",  1, 255, -1, 0, nullptr, 0 }, 
	{ 456, "301Z011y", 1, 255, -1, 0, nullptr, 0 }, 
	{ 488, "301Z011z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 526, "301_021",  2, 200, -1, 0, nullptr, 0 }, 
	{ 542, "301Z007",  1, 150, -1, 0, nullptr, 0 }, 
	{ 549, "301w999",  2, 150, -1, 0, nullptr, 0 }, 
	{ 564, "301Z008",  1, 255, -1, 0, nullptr, 0 }, 
	{ 591, "301W004",  1, 255, -1, 0, nullptr, 0 }, 
	{ 620, "301Z009y", 1, 255, -1, 0, nullptr, 0 }, 
	{ 631, "301Z009z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 688, "301Z010x", 1, 255, -1, 0, nullptr, 0 }, 
	{ 700, "301Z010y", 1, 255, -1, 0, nullptr, 0 }, 
	{ 747, "301Z010z", 1, 255, -1, 0, nullptr, 0 }, 
	{ 792, "301_009",  2, 255, -1, 0, nullptr, 0 }, 
	{ 795, "301_003",  1, 255, -1, 0, nullptr, 0 }, 
	{ 803, "301_002",  2, 255, -1, 0, nullptr, 0 }, 
	{ 812, "301_011",  2, 255, -1, 0, nullptr, 0 }, 
	{ 821, "301_005",  1, 255, -1, 0, nullptr, 0 }, 
	{ 826, "301_018",  2, 175, -1, 0, nullptr, 0 }, 
	{ 835, nullptr,    1,   0, -1, 0, nullptr, 0 },
	{ 836, nullptr,    0,   0, 15, 0, nullptr, 0 },
	{ 837, nullptr,    0,   0, 20, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room301::SERIES3[] = {
	{   0, "301_004", 3, 100, -1, (uint)-1, nullptr, 0 },
	{   0, "301_010", 2,  30, -1, (uint)-1, nullptr, 0 },
	{   0, nullptr,   0,   0, 18,  0, nullptr, 0 },
	{   1, "301Z101", 1, 255, -1,  0, nullptr, 0 }, 
	{  30, "301W101", 1, 255, -1,  0, nullptr, 0 }, 
	{  49, "301Z102", 1, 255, -1,  0, nullptr, 0 }, 
	{  96, "301W102", 1, 255, -1,  0, nullptr, 0 }, 
	{ 127, nullptr,   0,   0, 15,  0, nullptr, 0 },
	{ 130, nullptr,   3,   0, -1,  0, nullptr, 0 },
	{ 131, nullptr,   0,   0, 20,  0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room301::SERIES4[] = {
	{  0, "301_010", 2,  30, -1, (uint)-1, nullptr, 0 },
	{  0, nullptr,   0,   0, 18,  0, nullptr, 0 },
	{  1, "301_004", 3, 255, -1, (uint)-1, nullptr, 0 },
	{ 37, nullptr,   0,   0, 15,  0, nullptr, 0 },
	{ 38, nullptr,   0,   0, 20,  0, nullptr, 0 },
	{ -1, nullptr,   0,   0, -1,  0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room301::init() {
}

void Room301::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
