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

#include "m4/burger/rooms/section4/room401.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room401::SERIES1[] = {
	{  7, "401v001",  1, 255, -1, 0, 0, 0 },
	{ 36, "401A_002", 2, 200, -1, 0, 0, 0 },
	{ 89, nullptr,    0,   0,  9, 0, 0, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room401::SERIES2[] = {
	{  0, "401B_003", 2, 200, -1, 0, nullptr, 0 },
	{  41, "401Z001", 1, 255, -1, 0, nullptr, 0 },
	{  69, "401Z002", 1, 255, -1, 0, nullptr, 0 },
	{  99, "401V002", 1, 255, -1, 0, nullptr, 0 },
	{ 162, "401Z003", 1, 255, -1, 0, nullptr, 0 },
	{ 228, "401V003", 1, 255, -1, 0, nullptr, 0 },
	{ 319, "401Z004", 1, 255, -1, 0, nullptr, 0 },
	{ 386, "401V004", 1, 255, -1, 0, nullptr, 0 },
	{ 417, "401V005", 1, 255, -1, 0, nullptr, 0 },
	{ 461, "401Z005", 1, 255, -1, 0, nullptr, 0 },
	{ 567, "401V006", 1, 255, -1, 0, nullptr, 0 },
	{ 582, "401Z006", 1, 255, -1, 0, nullptr, 0 },
	{ 661, "401V007", 1, 255, -1, 0, nullptr, 0 },
	{ 758, nullptr,   0,   0,  9, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room401::SERIES3[] = {
	{   0, "401C_001", 3, 150, -1, (uint)-1, nullptr, 0 },
	{  19, "401z007",  1, 255, -1,  0, nullptr, 0 },
	{  45, "401f001",  1, 255, -1,  0, nullptr, 0 },
	{  63, "401z008",  1, 255, -1,  0, nullptr, 0 },
	{ 100, nullptr,    0,   0,  8,  0, nullptr, 0 },
	{ 102, "401f002",  1, 255, -1,  0, nullptr, 0 },
	{ 104, nullptr,    0,   0,  9,  0, nullptr, 0 },
	STREAM_BREAK_END
};

void Room401::init() {
}

void Room401::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
